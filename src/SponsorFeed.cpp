#include "SponsorFeed.h"
#include "SponsorConfig.h"
#include "SponsorDocument.h"
#include <QNetworkDiskCache>
#include <QStandardPaths>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QImageReader>
#include <QBuffer>
#include <QDesktopServices>
#include <memory>
SponsorFeed::SponsorFeed(QObject* p):QObject(p){
 current_={{"name","CloudBox"},{"titleTr","Dosyalarınız her zaman güvende!"},{"titleEn","Your files. Always protected."},{"bodyTr","Kurgusal sponsor örneği. Veri toplanmaz."},{"bodyEn","Fictional sponsor sample. No data collection."},{"link",""}};
 feed_=QUrl(QSettings().value("sponsors/feedUrl",QString::fromLatin1(fm::sponsor::defaultUrl)).toString());
 auto* cache=new QNetworkDiskCache(&network_);cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/sponsors");cache->setMaximumCacheSize(20000000);network_.setCache(cache);
 if(feed_.scheme()!="https"||feed_.host().isEmpty()||!feed_.userInfo().isEmpty())feed_=QUrl();
 timer_.setInterval(900000);connect(&timer_,&QTimer::timeout,this,&SponsorFeed::refresh);timer_.start();
 QTimer::singleShot(0,this,&SponsorFeed::refresh);
}
void SponsorFeed::pause(bool value){if(paused_==value)return;paused_=value;if(value&&reply_)reply_->abort();if(!value)refresh();}
void SponsorFeed::refresh(){if(paused_||reply_||feed_.isEmpty())return;fetch(feed_,false);}
void SponsorFeed::fetch(const QUrl& url,bool image){
 QNetworkRequest request(url);request.setTransferTimeout(10000);
 request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,image?QNetworkRequest::PreferCache:QNetworkRequest::AlwaysNetwork);
 if(!image&&url.host()=="mesqqxlacjuvinsdospy.supabase.co"&&url.path()=="/rest/v1/freemotra_ads")request.setRawHeader("apikey",fm::sponsor::publishableKey);
 request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);
 request.setAttribute(QNetworkRequest::CookieLoadControlAttribute,QNetworkRequest::Manual);
 request.setAttribute(QNetworkRequest::CookieSaveControlAttribute,QNetworkRequest::Manual);
 auto response=network_.get(request);reply_=response;
 auto bytes=std::make_shared<QByteArray>();const qsizetype limit=image?1000000:64000;
 connect(response,&QNetworkReply::readyRead,this,[response,bytes,limit]{*bytes+=response->readAll();if(bytes->size()>limit)response->abort();});
 connect(response,&QNetworkReply::finished,this,[this,response,bytes,image,limit]{
  *bytes+=response->readAll();reply_=nullptr;
  const bool valid=!paused_&&response->error()==QNetworkReply::NoError&&response->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200&&bytes->size()<=limit;
  response->deleteLater();if(!valid)return;
  if(image){QBuffer buffer(bytes.get());buffer.open(QIODevice::ReadOnly);QImageReader reader(&buffer,"PNG");const auto size=reader.size();if(size.width()>0&&size.height()>0&&size.width()<=1600&&size.height()<=1600&&!reader.read().isNull()){image_="data:image/png;base64,"+QString::fromLatin1(bytes->toBase64());emit changed();}return;}
  QJsonObject obj;bool empty=false;if(!fm::sponsor::parse(*bytes,obj,empty))return;
  if(empty){available_=false;image_.clear();emit changed();return;}
  QVariantMap clean;
  for(const auto& name:{"name","titleTr","titleEn","bodyTr","bodyEn","link"}){const auto text=obj[name].toString();if(text.size()>1024)return;clean.insert(name,text);}
  const QUrl link(clean["link"].toString());if(!link.isEmpty()&&(link.scheme()!="https"||link.host().isEmpty()||!link.userInfo().isEmpty()))return;
  current_=clean;available_=true;sample_=false;image_.clear();emit changed();
  const QUrl picture(obj["image"].toString());
  if(fm::sponsor::trustedImage(picture,feed_))fetch(picture,true);
 });
}
void SponsorFeed::openLink(){const QUrl url(current_["link"].toString());if(!paused_&&url.scheme()=="https"&&!url.host().isEmpty()&&url.userInfo().isEmpty())QDesktopServices::openUrl(url);}
