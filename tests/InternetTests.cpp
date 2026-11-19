#include "LanLink.h"
#include "SignalCrypto.h"
#include <QtTest>
class InternetTests:public QObject {
 Q_OBJECT
private slots:
 void authenticatedEnvelopes(){
  const auto key=fm::signal::random(32);QJsonObject out;const QString room=QString(32,'a'),sender=QString(32,'b');
  const QJsonObject secret{{"type","auth"},{"password","NEVER-PLAINTEXT"}};const auto e=fm::signal::seal(key,room,sender,1,secret);
  QVERIFY(!QJsonDocument(e).toJson().contains("NEVER-PLAINTEXT"));QVERIFY(fm::signal::open(key,room,e,out));QCOMPARE(out,secret);
  QVERIFY(!fm::signal::open(fm::signal::random(32),room,e,out));QVERIFY(!fm::signal::open(key,QString(32,'c'),e,out));
  auto tampered=e;tampered["seq"]="2";QVERIFY(!fm::signal::open(key,room,tampered,out));
  tampered=e;tampered["sender"]=QString(32,'c');QVERIFY(!fm::signal::open(key,room,tampered,out));
  tampered=e;auto box=QByteArray::fromBase64(e["box"].toString().toLatin1());box[box.size()-1]^=1;tampered["box"]=QString::fromLatin1(box.toBase64());QVERIFY(!fm::signal::open(key,room,tampered,out));
 }
 void invitationBoundary(){QString room;QByteArray key;QVERIFY(fm::signal::invitation("fm3://"+QString(32,'a')+"/"+QString(64,'b'),room,key));QCOMPARE(key.size(),32);QVERIFY(!fm::signal::invitation("https://evil.example",room,key));QVERIFY(!fm::signal::invitation("fm3://"+QString(32,'a')+"/"+QString(64,'b')+"?extra",room,key));}
 void liveSignaling(){
  if(!qEnvironmentVariableIsSet("FREEMOTRA_LIVE_INTERNET_TEST"))QSKIP("Live service test runs explicitly on Windows CI");
  InternetSignal host,client;QSignalSpy hReady(&host,&InternetSignal::ready),cReady(&client,&InternetSignal::ready),received(&host,&InternetSignal::message);
  const auto invite=host.host();QVERIFY(!invite.isEmpty());QTRY_COMPARE_WITH_TIMEOUT(hReady.size(),1,30000);
  QVERIFY(client.join(invite));QTRY_COMPARE_WITH_TIMEOUT(cReady.size(),1,30000);QVERIFY(client.send({{"type","probe"},{"text","encrypted-live-test"}}));
  QTRY_COMPARE_WITH_TIMEOUT(received.size(),1,10000);QCOMPARE(received[0][1].toJsonObject()["text"].toString(),QString("encrypted-live-test"));
 }
 void liveInternetPeer(){
  if(!qEnvironmentVariableIsSet("FREEMOTRA_LIVE_INTERNET_TEST"))QSKIP("Live service test runs explicitly on Windows CI");
  rtc::InitLogger(rtc::LogLevel::Info);
  LanLink host,client;
  connect(&host,&LanLink::status,this,[](const QString& s){qInfo().noquote()<<"Host:"<<s;});
  connect(&client,&LanLink::status,this,[](const QString& s){qInfo().noquote()<<"Viewer:"<<s;});
  host.verifyPassword=[](const QString& password){return password=="live-test-password";};
  QSignalSpy request(&host,&LanLink::request),received(&host,&LanLink::message);
  const auto identity="FM-"+QString::fromLatin1(fm::signal::random(16).toHex().toUpper());
  const auto invite=host.listenInternet(identity,"live-test-password");QVERIFY(!invite.isEmpty());QTRY_VERIFY_WITH_TIMEOUT(host.internetReady(),30000);client.dial(identity,"live-test-password","CI viewer");
  QTRY_COMPARE_WITH_TIMEOUT(request.size(),1,30000);qInfo()<<"Authenticated, approving";QVERIFY(!host.active());QVERIFY(!client.active());host.approve(false);
  QElapsedTimer wait;wait.start();while(!(host.active()&&client.active())&&wait.elapsed()<95000)QTest::qWait(50);QVERIFY2(host.active()&&client.active(),"WebRTC did not open within 95 seconds");QVERIFY(!host.controlAllowed());client.send("internet-verified");
  QTRY_COMPARE_WITH_TIMEOUT(received.size(),1,10000);QCOMPARE(received[0][0].toByteArray(),QByteArray("internet-verified"));
  qInfo()<<"Verified data, disconnecting";client.stop();QTRY_VERIFY_WITH_TIMEOUT(!host.active(),20000);
 }
 void liveConfiguredRelay(){
  if(!qEnvironmentVariableIsSet("FREEMOTRA_TEST_CONFIGURED_RELAY"))QSKIP("Owner-configured relay test enabled explicitly");
  InternetSignal probe;QSignalSpy ready(&probe,&InternetSignal::ready);probe.host();QTRY_COMPARE_WITH_TIMEOUT(ready.size(),1,30000);const bool configured=!probe.iceServers().isEmpty();probe.stop();
  if(!configured)QSKIP("TURN is not configured: direct internet works; restrictive-network relay is NOT verified");
  LanLink host,client;host.relayOnly=client.relayOnly=true;host.verifyPassword=[](const QString& p){return p=="relay-test";};QSignalSpy request(&host,&LanLink::request),data(&host,&LanLink::message),hostEnded(&host,&LanLink::ended),clientEnded(&client,&LanLink::ended);
  const auto invite=host.listenInternet();QTRY_VERIFY_WITH_TIMEOUT(host.internetReady(),30000);client.dial(invite,"relay-test","CI relay probe");QTRY_COMPARE_WITH_TIMEOUT(request.size(),1,30000);hostEnded.clear();clientEnded.clear();host.approve(false);
  QTRY_VERIFY_WITH_TIMEOUT((host.active()&&client.active())||!hostEnded.isEmpty()||!clientEnded.isEmpty(),95000);QVERIFY2(host.active()&&client.active(),"Configured TURN did not establish a relay connection");client.send("forced-relay-verified");QTRY_COMPARE_WITH_TIMEOUT(data.size(),1,10000);QCOMPARE(data[0][0].toByteArray(),QByteArray("forced-relay-verified"));
 }
};
QTEST_GUILESS_MAIN(InternetTests)
#include "InternetTests.moc"
