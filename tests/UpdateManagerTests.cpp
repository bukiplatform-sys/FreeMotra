#include "UpdateManager.h"
#include <QtTest>
#include <cstring>
#include <QStandardPaths>
class FakeReply final:public QNetworkReply {
 QByteArray bytes_;qint64 offset_=0;bool done_=false;
public:
 FakeReply(QByteArray bytes,int code,QObject* parent):QNetworkReply(parent),bytes_(std::move(bytes)) {
  setAttribute(QNetworkRequest::HttpStatusCodeAttribute,code);open(QIODevice::ReadOnly);
  QTimer::singleShot(0,this,[this]{if(done_)return;emit readyRead();if(done_)return;done_=true;setFinished(true);emit finished();});
 }
 void abort()override{if(done_)return;done_=true;setError(OperationCanceledError,"cancelled");setFinished(true);emit finished();}
 bool isSequential()const override{return true;}
 qint64 bytesAvailable()const override{return bytes_.size()-offset_+QNetworkReply::bytesAvailable();}
protected:
 qint64 readData(char* data,qint64 max)override{const auto n=qMin(max,qint64(bytes_.size())-offset_);if(!n)return -1;std::memcpy(data,bytes_.constData()+offset_,size_t(n));offset_+=n;return n;}
};
class FakeUpdater final:public UpdateManager {
public:
 QByteArray body;int code=200,calls=0;
 FakeUpdater():UpdateManager(false){}
 QNetworkReply* get(const QUrl&,bool)override{++calls;return new FakeReply(body,code,this);}
};
class UpdateManagerTests:public QObject {
 Q_OBJECT
 QByteArray payload(){QByteArray b(2048,'x');b[0]='M';b[1]='Z';return b;}
 void prepare(FakeUpdater& updater){updater.release_={"0.4.0","notes",QUrl("https://github.com/bukiplatform-sys/FreeMotra/releases/download/v0.4.0/FreeMotra-0.4.0-windows-x64.exe"),QCryptographicHash::hash(payload(),QCryptographicHash::Sha256),2048};updater.body=payload();updater.setState("available");}
private slots:
 void initTestCase(){QCoreApplication::setApplicationName("FreeMotra-UpdateTests");QCoreApplication::setApplicationVersion("0.3.2");QStandardPaths::setTestModeEnabled(true);}
 void downloadAndInstallGuards(){
  FakeUpdater updater;prepare(updater);updater.download();QTRY_COMPARE(updater.state(),QString("ready"));
  QVERIFY(QFile::exists(updater.path_));QCOMPARE(updater.progress(),1.0);
  updater.check();QCOMPARE(updater.calls,1);
  updater.setBlocked(true);updater.install();QCOMPARE(updater.state(),QString("ready"));
  updater.setBlocked(false);QFile file(updater.path_);QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write(payload().replace('x','y'));file.close();
  updater.install();QCOMPARE(updater.state(),QString("error"));
 }
 void damagedAndTruncatedDownloads(){
  for(int kind=0;kind<4;++kind){FakeUpdater updater;prepare(updater);
   if(kind==0)updater.body[100]='y';if(kind==1)updater.body.chop(10);if(kind==2)updater.body.append('x');if(kind==3)updater.code=503;
   updater.download();QTRY_COMPARE(updater.state(),QString("error"));QVERIFY(!QFile::exists(updater.path_));
  }
 }
 void cancellationAndRetry(){
  FakeUpdater updater;prepare(updater);updater.download();updater.cancel();QCOMPARE(updater.state(),QString("available"));QVERIFY(!QFile::exists(updater.path_));
  updater.download();QTRY_COMPARE(updater.state(),QString("ready"));
 }
 void metadataFailureKeepsKnownUpdate(){
  FakeUpdater updater;prepare(updater);updater.body="{}";updater.check();QTRY_COMPARE(updater.state(),QString("error"));QVERIFY(updater.available());
 }
 void shutdownDuringDownload(){auto updater=std::make_unique<FakeUpdater>();prepare(*updater);updater->download();updater.reset();QCoreApplication::processEvents();}
 void publishedReleaseDownload(){
  if(!qEnvironmentVariableIsSet("FREEMOTRA_LIVE_UPDATE_TEST"))QSKIP("Runs after publication on Windows CI");
  QCoreApplication::setApplicationVersion("0.0.0");
  UpdateManager updater(false);updater.check();
  QTRY_COMPARE_WITH_TIMEOUT(updater.state(),QString("available"),60000);
  updater.download();QTRY_COMPARE_WITH_TIMEOUT(updater.state(),QString("ready"),180000);
  QCOMPARE(updater.progress(),1.0);QVERIFY(QFile::exists(updater.path_));
  updater.setBlocked(true);updater.install();QCOMPARE(updater.state(),QString("ready"));
 }
};
QTEST_GUILESS_MAIN(UpdateManagerTests)
#include "UpdateManagerTests.moc"
