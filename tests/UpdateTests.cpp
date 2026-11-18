#include "UpdateRelease.h"
#include <QtTest>
class UpdateTests:public QObject {
 Q_OBJECT
 QJsonObject release(){return {{"tag_name","v0.4.0"},{"draft",false},{"prerelease",false},{"body","Release notes"},{"assets",QJsonArray{QJsonObject{{"name","FreeMotra-0.4.0-windows-x64.exe"},{"state","uploaded"},{"size",2048},{"digest","sha256:"+QString(64,'a')},{"browser_download_url","https://github.com/bukiplatform-sys/FreeMotra/releases/download/v0.4.0/FreeMotra-0.4.0-windows-x64.exe"}}}}};}
private slots:
 void versions(){
  QVERIFY(fm::update::newer("0.10.0","0.9.9"));QVERIFY(!fm::update::newer("0.3.2","0.3.2"));QVERIFY(!fm::update::newer("0.3.1","0.3.2"));
  for(const auto& v:{"01.2.3","1.2","1.2.3-beta","1.2.3.exe","1.2.3/evil","9999999.0.0"})QVERIFY(!fm::update::versionValid(v));
 }
 void releaseBoundary(){
  fm::update::Release out;auto o=release();QVERIFY(fm::update::parse(QJsonDocument(o).toJson(),out));QCOMPARE(out.version,QString("0.4.0"));QCOMPARE(out.hash,QByteArray(32,char(0xaa)));
  for(const auto& key:{"draft","prerelease"}){auto bad=o;bad[key]=true;QVERIFY(!fm::update::parse(QJsonDocument(bad).toJson(),out));}
  for(const auto& url:{"http://github.com/file.exe","https://evil.example/file.exe","https://github.com/other/repo/releases/download/v0.4.0/file.exe"}){auto bad=o;auto a=bad["assets"].toArray();auto b=a[0].toObject();b["browser_download_url"]=url;a[0]=b;bad["assets"]=a;QVERIFY(!fm::update::parse(QJsonDocument(bad).toJson(),out));}
  for(const auto& key:{"digest","size","state"}){auto bad=o;auto a=bad["assets"].toArray();auto b=a[0].toObject();b.remove(key);a[0]=b;bad["assets"]=a;QVERIFY(!fm::update::parse(QJsonDocument(bad).toJson(),out));}
  auto assets=o["assets"].toArray();assets.append(assets[0]);o["assets"]=assets;QVERIFY(!fm::update::parse(QJsonDocument(o).toJson(),out));
  QVERIFY(!fm::update::parse("{}",out));QVERIFY(!fm::update::parse(QByteArray(1048577,'x'),out));
 }
 void redirects(){
  QVERIFY(fm::update::downloadHost(QUrl("https://release-assets.githubusercontent.com/file?token=example")));
  for(const auto& url:{"http://github.com/file","https://github.com.evil.example/file","https://user:pass@github.com/file","https://github.com:444/file","https://evil.example/file"})QVERIFY(!fm::update::downloadHost(QUrl(url)));
 }
};
QTEST_GUILESS_MAIN(UpdateTests)
#include "UpdateTests.moc"
