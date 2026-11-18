#pragma once
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUrl>
namespace fm::sponsor {
inline bool parse(const QByteArray& bytes,QJsonObject& selected,bool& empty){
 const auto document=QJsonDocument::fromJson(bytes);QJsonArray list;bool rows=false;
 if(document.isArray()){list=document.array();rows=true;}
 else if(document.isObject()){const auto obj=document.object();if(obj["version"].toInt()!=1||!obj["sponsors"].isArray())return false;list=obj["sponsors"].toArray();}
 else return false;
 if(list.size()>20)return false;empty=list.isEmpty();if(empty)return true;
 if(!list.first().isObject())return false;selected=list.first().toObject();
 if(rows){if(!selected["payload"].isObject())return false;selected=selected["payload"].toObject();}
 for(const auto* name:{"name","titleTr","titleEn","bodyTr","bodyEn","link","image"})if(!selected[name].isString()||selected[name].toString().size()>1024)return false;
 if(selected["name"].toString().isEmpty()||selected["titleTr"].toString().isEmpty())return false;
 const QUrl link(selected["link"].toString());return link.isEmpty()||(link.scheme()=="https"&&!link.host().isEmpty()&&link.userInfo().isEmpty());
}
inline bool trustedImage(const QUrl& image,const QUrl& feed){
 if(image.scheme()!="https"||image.host()!=feed.host()||image.port()!=feed.port()||!image.userInfo().isEmpty()||!image.query().isEmpty()||!image.fragment().isEmpty())return false;
 return QRegularExpression("^(/images/|/storage/v1/object/public/freemotra-sponsor-images/)[a-f0-9]{64}\\.png$").match(image.path()).hasMatch();
}
}
