#include "SponsorDocument.h"
#include <QtTest>
class SponsorTests:public QObject {
 Q_OBJECT
private slots:
 void feedFormats(){
  QJsonObject payload{{"name","Sponsor"},{"titleTr","Baslik"},{"titleEn",""},{"bodyTr",""},{"bodyEn",""},{"link","https://example.com"},{"image",""}},selected;bool empty=false;
  QVERIFY(fm::sponsor::parse(QJsonDocument(QJsonArray{QJsonObject{{"payload",payload}}}).toJson(),selected,empty));QVERIFY(!empty);QCOMPARE(selected,payload);
  QVERIFY(fm::sponsor::parse(QJsonDocument(QJsonObject{{"version",1},{"sponsors",QJsonArray{payload}}}).toJson(),selected,empty));
  QVERIFY(fm::sponsor::parse("[]",selected,empty));QVERIFY(empty);
  payload["link"]="javascript:alert(1)";QVERIFY(!fm::sponsor::parse(QJsonDocument(QJsonArray{QJsonObject{{"payload",payload}}}).toJson(),selected,empty));
  QVERIFY(!fm::sponsor::parse("[{\"payload\":{}}]",selected,empty));
 }
 void imageBoundary(){
  const QUrl feed("https://project.supabase.co/rest/v1/freemotra_ads");const QString path="/storage/v1/object/public/freemotra-sponsor-images/"+QString(64,'a')+".png";
  QVERIFY(fm::sponsor::trustedImage(QUrl("https://project.supabase.co"+path),feed));
  QVERIFY(!fm::sponsor::trustedImage(QUrl("https://evil.example"+path),feed));
  QVERIFY(!fm::sponsor::trustedImage(QUrl("https://project.supabase.co"+path+"?x=1"),feed));
  QVERIFY(!fm::sponsor::trustedImage(QUrl("https://project.supabase.co/storage/v1/object/public/another/"+QString(64,'a')+".png"),feed));
 }
};
QTEST_GUILESS_MAIN(SponsorTests)
#include "SponsorTests.moc"
