#include "SessionTools.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QDir>
class SessionTests final: public QObject {
 Q_OBJECT
private:
 bool open(LanLink& host,LanLink& client,bool files=true,bool chat=true,bool voice=true){
  host.verifyPassword=[](const QString& p){return p=="test";};
  connect(&host,&LanLink::request,&host,[&host,files,chat,voice](const QString&){host.approve(true,false,files,chat,voice);});
  QSignalSpy status(&host,&LanLink::status),clientStatus(&client,&LanLink::status);
  const auto invite=host.listen(false,true);if(invite.isEmpty()){if(!status.isEmpty())qWarning()<<status.last();return false;}
  client.dial(invite,"test","viewer");
  const bool connected=QTest::qWaitFor([&]{return host.active()&&client.active();},25000);
  if(!connected){if(!status.isEmpty())qWarning()<<"Host:"<<status.last();if(!clientStatus.isEmpty())qWarning()<<"Client:"<<clientStatus.last();}
  return connected;
 }
 static QByteArray packet(const QJsonObject& o){return "T"+QJsonDocument(o).toJson(QJsonDocument::Compact);}
 static void wire(LanLink& host,SessionTools& h,LanLink& client,SessionTools& c){
  connect(&host,&LanLink::message,&h,&SessionTools::receive);
  connect(&client,&LanLink::message,&c,&SessionTools::receive);
 }
private slots:
 void capabilitiesAndChat(){
  LanLink host,client;SessionTools h(host),c(client);wire(host,h,client,c);QVERIFY(open(host,client));
  QVERIFY(host.active());QVERIFY(!host.viewAllowed());QVERIFY(!client.viewAllowed());
  QVERIFY(!host.controlAllowed());QVERIFY(!client.controlAllowed());QVERIFY(client.filesAllowed());QVERIFY(client.voiceAllowed());
  c.sendChat("Merhaba <b>Sonat</b>");QTRY_COMPARE(h.messages().size(),1);QCOMPARE(h.messages().first().toMap()["text"].toString(),QString("Merhaba <b>Sonat</b>"));
  h.sendChat("Hello");QTRY_COMPARE(c.messages().size(),2);
  QVERIFY(!h.microphone());QVERIFY(!c.microphone());QVERIFY(!h.listening());
  client.stop();QTRY_VERIFY(!host.active());QVERIFY(h.messages().isEmpty());QVERIFY(!h.filesAllowed());
 }
 void deniedCapabilities(){
  LanLink host,client;SessionTools h(host),c(client);wire(host,h,client,c);QVERIFY(open(host,client,false,false,false));
  QVERIFY(host.active());c.sendChat("denied");client.send("Cforged");
  client.send(packet({{"op","offer"},{"id","00000000-0000-0000-0000-000000000001"},{"name","a.txt"},{"size","0"},{"hash",QString(64,'a')}}));
  h.setMicrophone(true);h.setListening(true);QTest::qWait(200);
  QVERIFY(h.messages().isEmpty());QVERIFY(h.pendingFile().isEmpty());QVERIFY(!h.microphone());QVERIFY(!h.listening());
 }
 void fileTransferResumeAndNames(){
  QTemporaryDir source,destination;QVERIFY(source.isValid());QVERIFY(destination.isValid());
  QByteArray content;for(int i=0;i<100000;++i)content.append(char(i%251));
  const auto path=source.filePath("music.dat");QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly));QCOMPARE(file.write(content),qint64(content.size()));file.close();
  const auto hash=QString::fromLatin1(QCryptographicHash::hash(content,QCryptographicHash::Sha256).toHex());
  const auto partialPath=destination.filePath(".freemotra-"+hash+".part");
  LanLink host,client;SessionTools h(host),c(client);wire(host,h,client,c);QVERIFY(open(host,client));
  // Stop a real transfer after the first chunk; resume the remaining bytes in a new offer.
  bool interrupt=true;
  connect(&h,&SessionTools::changed,&h,[&]{if(interrupt&&h.progress()>0&&h.progress()<1){interrupt=false;h.cancelTransfer();}});
  c.sendFile(QUrl::fromLocalFile(path));QTRY_COMPARE(h.pendingFile(),QString("music.dat"));
  QVERIFY(!QFileInfo::exists(partialPath)); // No disk writes before local acceptance.
  h.acceptFile(QUrl::fromLocalFile(destination.path()));QTRY_VERIFY(!interrupt);
  QTRY_VERIFY(c.transferStatus().contains("Peer cancelled"));
  QVERIFY(QFileInfo(partialPath).size()>0);QVERIFY(QFileInfo(partialPath).size()<content.size());
  c.sendFile(QUrl::fromLocalFile(path));QTRY_COMPARE(h.pendingFile(),QString("music.dat"));h.acceptFile(QUrl::fromLocalFile(destination.path()));
  QTRY_VERIFY_WITH_TIMEOUT(c.transferStatus().contains("verified by receiver"),15000);
  QFile received(destination.filePath("music.dat"));QVERIFY(received.open(QIODevice::ReadOnly));QCOMPARE(received.readAll(),content);received.close();QVERIFY(!QFileInfo::exists(partialPath));
  // Do not overwrite a file that is already present.
  c.sendFile(QUrl::fromLocalFile(path));QTRY_COMPARE(h.pendingFile(),QString("music.dat"));h.acceptFile(QUrl::fromLocalFile(destination.path()));QVERIFY(h.transferStatus().contains("Destination exists"));h.rejectFile();QTRY_VERIFY(c.transferStatus().contains("Peer cancelled"));
  for(const auto& name:QStringList{"../escape","C:\\escape","file:stream","CON.txt","trailing.","a/b"}){
   client.send(packet({{"op","offer"},{"id","00000000-0000-0000-0000-000000000001"},{"name",name},{"size","0"},{"hash",hash}}));
  }
  QTest::qWait(300);QVERIFY(h.pendingFile().isEmpty());
 }
 void corruptPartialAndEmptyFile(){
  QTemporaryDir source,destination;
  const auto path=source.filePath("small.txt");QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly));file.write("correct");file.close();
  const auto hash=QString::fromLatin1(QCryptographicHash::hash("correct",QCryptographicHash::Sha256).toHex());
  QFile partial(destination.filePath(".freemotra-"+hash+".part"));QVERIFY(partial.open(QIODevice::WriteOnly));partial.write("wrong");partial.close();
  LanLink host,client;SessionTools h(host),c(client);wire(host,h,client,c);QVERIFY(open(host,client));
  c.sendFile(QUrl::fromLocalFile(path));QTRY_VERIFY(!h.pendingFile().isEmpty());h.acceptFile(QUrl::fromLocalFile(destination.path()));
  QTRY_VERIFY(h.transferStatus().contains("verification failed"));QVERIFY(!QFileInfo::exists(destination.filePath("small.txt")));QVERIFY(!QFileInfo::exists(partial.fileName()));
  QTRY_VERIFY(c.transferStatus().contains("Peer cancelled"));
  QFile empty(source.filePath("empty.dat"));QVERIFY(empty.open(QIODevice::WriteOnly));empty.close();
  c.sendFile(QUrl::fromLocalFile(empty.fileName()));QTRY_COMPARE(h.pendingFile(),QString("empty.dat"));h.acceptFile(QUrl::fromLocalFile(destination.path()));
  QTRY_VERIFY(c.transferStatus().contains("verified by receiver"));QCOMPARE(QFileInfo(destination.filePath("empty.dat")).size(),qint64(0));
 }
};
QTEST_GUILESS_MAIN(SessionTests)
#include "SessionTests.moc"
