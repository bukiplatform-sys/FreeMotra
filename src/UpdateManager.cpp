#include "UpdateManager.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QFile>
UpdateManager::UpdateManager(bool enabled,QObject* parent):QObject(parent) {
 timer_.setInterval(6*60*60*1000);connect(&timer_,&QTimer::timeout,this,&UpdateManager::check);
 if(enabled){timer_.start();QTimer::singleShot(5000,this,&UpdateManager::check);}
}
QString UpdateManager::currentVersion()const{return QCoreApplication::applicationVersion();}
UpdateManager::~UpdateManager(){if(reply_){disconnect(reply_,nullptr,this,nullptr);reply_->abort();}}
QNetworkReply* UpdateManager::get(const QUrl& url,bool installer) {
 QNetworkRequest request(url);request.setRawHeader("User-Agent","FreeMotra/"+currentVersion().toUtf8());
 request.setRawHeader("Accept",installer?"application/octet-stream":"application/vnd.github+json");
 request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::UserVerifiedRedirectPolicy);
 request.setMaximumRedirectsAllowed(5);request.setTransferTimeout(30000);
 auto* r=network_.get(request);r->setReadBufferSize(65536);
 connect(r,&QNetworkReply::redirected,this,[r,installer](const QUrl& target){
  const bool api=target.scheme()=="https"&&target.host()=="api.github.com"&&target.userInfo().isEmpty()&&target.port(443)==443&&target.path()=="/repos/bukiplatform-sys/FreeMotra/releases/latest";
  if(installer?fm::update::downloadHost(target):api)r->redirectAllowed();else r->abort();
 });return r;
}
void UpdateManager::check() {
 if(reply_||state_=="ready"||state_=="launching")return;
 metadata_.clear();setState("checking");auto* r=get(QUrl("https://api.github.com/repos/bukiplatform-sys/FreeMotra/releases/latest"),false);reply_=r;
 connect(r,&QIODevice::readyRead,this,[this,r]{metadata_+=r->readAll();if(metadata_.size()>1048576)r->abort();});
 connect(r,&QNetworkReply::finished,this,[this,r]{
  metadata_+=r->readAll();const bool ok=r->error()==QNetworkReply::NoError&&r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200;
  reply_=nullptr;r->deleteLater();fm::update::Release result;
  if(!ok||!fm::update::parse(metadata_,result)){setState("error");return;}
  release_=result;setState(available()?"available":"current");
 });
}
void UpdateManager::download() {
 if(reply_||!available()||state_=="ready"||state_=="launching")return;
 const auto base=QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/updates";
 if(!QDir().mkpath(base)){setState("error");return;}
 directory_=std::make_unique<QTemporaryDir>(base+"/install-XXXXXX");
 if(!directory_->isValid()){setState("error");return;}
 path_=directory_->filePath("FreeMotra-"+release_.version+"-setup.exe");
 file_=std::make_unique<QSaveFile>(path_);if(!file_->open(QIODevice::WriteOnly)){setState("error");return;}
 cancelled_=writeFailed_=false;received_=0;hash_.reset();setState("downloading");
 auto* r=get(release_.url,true);reply_=r;
 const auto consume=[this,r]{
  const auto data=r->readAll();received_+=data.size();
  if(received_>release_.size||file_->write(data)!=data.size()){writeFailed_=true;r->abort();return;}
  hash_.addData(data);emit changed();
 };
 connect(r,&QIODevice::readyRead,this,consume);
 connect(r,&QNetworkReply::finished,this,[this,r,consume]{
  consume();const bool ok=!cancelled_&&!writeFailed_&&r->error()==QNetworkReply::NoError&&r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200&&received_==release_.size&&hash_.result()==release_.hash;
  reply_=nullptr;r->deleteLater();
  if(!ok){file_->cancelWriting();file_.reset();directory_.reset();setState(cancelled_?"available":"error");return;}
  if(!file_->commit()){file_.reset();directory_.reset();setState("error");return;}
  file_.reset();setState("ready");
 });
}
void UpdateManager::cancel(){if(state_=="downloading"&&reply_){cancelled_=true;reply_->abort();}}
void UpdateManager::install() {
 if(blocked_||state_!="ready")return;
 QFile file(path_);QCryptographicHash hash(QCryptographicHash::Sha256);
 if(!file.open(QIODevice::ReadOnly)||file.size()!=release_.size||file.read(2)!="MZ"||!file.seek(0)||!hash.addData(&file)||hash.result()!=release_.hash){setState("error");return;}
 file.close();
 // The user explicitly confirms in QML. Never pass silent-install arguments.
 if(!QProcess::startDetached(path_,QStringList{})){setState("error");return;}
 directory_->setAutoRemove(false);setState("launching");QCoreApplication::quit();
}
