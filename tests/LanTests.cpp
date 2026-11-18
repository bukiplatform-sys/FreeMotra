#include "LanLink.h"
#include <QtTest>
class LanTests:public QObject {
 Q_OBJECT
private slots:
 void privateAddresses(){
  QVERIFY(LanLink::privateAddress(QHostAddress("192.168.1.1")));
  QVERIFY(LanLink::privateAddress(QHostAddress("172.31.1.1")));
  QVERIFY(!LanLink::privateAddress(QHostAddress("172.32.1.1")));
  QVERIFY(!LanLink::privateAddress(QHostAddress("8.8.8.8")));
 }
 void consentAndWebRtc(){
  LanLink host,client;host.verifyPassword=[](const QString& p){return p=="test-password";};
  auto invite=host.listen();QVERIFY(!invite.isEmpty());
  QSignalSpy request(&host,&LanLink::request),hostOpen(&host,&LanLink::opened),clientOpen(&client,&LanLink::opened),data(&host,&LanLink::message);
  client.dial(invite,"test-password","Test viewer");
  QTRY_COMPARE_WITH_TIMEOUT(request.count(),1,15000);
  QVERIFY(!host.active());QCOMPARE(hostOpen.count(),0);QCOMPARE(clientOpen.count(),0);
  client.send("forbidden-before-consent");QTest::qWait(100);QCOMPARE(data.count(),0);
  host.approve(false);
  QTRY_COMPARE_WITH_TIMEOUT(clientOpen.count(),1,20000);
  QTRY_COMPARE_WITH_TIMEOUT(hostOpen.count(),1,20000);
  QVERIFY(!host.controlAllowed());client.send("verified-probe");
  QTRY_COMPARE_WITH_TIMEOUT(data.count(),1,5000);
  QCOMPARE(data.first().first().toByteArray(),QByteArray("verified-probe"));
  client.stop();QTRY_VERIFY_WITH_TIMEOUT(!host.active(),5000);
 }
 void wrongPassword(){
  LanLink host,client;host.verifyPassword=[](const QString&){return false;};
  const auto invite=host.listen();QSignalSpy request(&host,&LanLink::request),ended(&client,&LanLink::ended);
  client.dial(invite,"wrong","viewer");QTRY_VERIFY_WITH_TIMEOUT(ended.count()>1,15000);QCOMPARE(request.count(),0);QVERIFY(!host.active());
 }
 void wrongPin(){
  LanLink host,client;host.verifyPassword=[](const QString&){return true;};
  auto invite=host.listen();invite[invite.size()-1]=invite.back()=='a'?'b':'a';
  QSignalSpy request(&host,&LanLink::request),ended(&client,&LanLink::ended);
  client.dial(invite,"test","viewer");QTRY_VERIFY_WITH_TIMEOUT(ended.count()>1,15000);QCOMPARE(request.count(),0);
 }
};
QTEST_GUILESS_MAIN(LanTests)
#include "LanTests.moc"
