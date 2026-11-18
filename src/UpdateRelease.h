#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QVersionNumber>
#include <QUrl>
namespace fm::update {
struct Release { QString version, notes; QUrl url; QByteArray hash; qint64 size=0; };
inline bool versionValid(const QString& v) { return QRegularExpression("^(0|[1-9][0-9]{0,5})\\.(0|[1-9][0-9]{0,5})\\.(0|[1-9][0-9]{0,5})$").match(v).hasMatch(); }
inline bool newer(const QString& remote,const QString& local) { return versionValid(remote)&&versionValid(local)&&QVersionNumber::fromString(remote)>QVersionNumber::fromString(local); }
inline bool downloadHost(const QUrl& u) {
 return u.scheme()=="https"&&u.userInfo().isEmpty()&&u.port(443)==443&&!u.hasFragment()&&
  (u.host()=="github.com"||u.host()=="release-assets.githubusercontent.com"||u.host()=="objects.githubusercontent.com");
}
inline bool parse(const QByteArray& bytes,Release& out) {
 out={};if(bytes.size()>1048576)return false;
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);
 if(error.error!=QJsonParseError::NoError||!doc.isObject())return false;
 const auto o=doc.object();if(o["draft"].toBool(true)||o["prerelease"].toBool(true))return false;
 const auto tag=o["tag_name"].toString();if(!tag.startsWith('v')||!versionValid(tag.mid(1)))return false;
 const auto name="FreeMotra-"+tag.mid(1)+"-windows-x64.exe";
 const auto expected="https://github.com/bukiplatform-sys/FreeMotra/releases/download/"+tag+"/"+name;
 int matches=0;Release candidate;
 for(const auto& value:o["assets"].toArray()) {
  const auto a=value.toObject();if(a["name"].toString()!=name)continue;
  if(++matches>1||a["state"].toString()!="uploaded"||a["browser_download_url"].toString()!=expected)return false;
  const auto digest=a["digest"].toString();const double size=a["size"].toDouble();
  if(!QRegularExpression("^sha256:[a-f0-9]{64}$").match(digest).hasMatch()||size<1024||size>157286400||size!=qint64(size))return false;
  candidate={tag.mid(1),o["body"].toString().left(12000),QUrl(expected),QByteArray::fromHex(digest.mid(7).toLatin1()),qint64(size)};
 }
 if(matches!=1)return false;out=candidate;return true;
}
}
