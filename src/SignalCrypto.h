#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <memory>
namespace fm::signal {
inline QByteArray random(int n){QByteArray b(n,Qt::Uninitialized);if(RAND_bytes(reinterpret_cast<unsigned char*>(b.data()),n)!=1)return {};return b;}
inline bool invitation(const QString& s,QString& room,QByteArray& key){
 const auto m=QRegularExpression("^fm3://([a-f0-9]{32})/([a-f0-9]{64})$").match(s.trimmed());if(!m.hasMatch())return false;
 room=m.captured(1);key=QByteArray::fromHex(m.captured(2).toLatin1());return key.size()==32;
}
inline QByteArray crypt(bool encrypt,const QByteArray& key,const QByteArray& nonce,const QByteArray& aad,const QByteArray& input,QByteArray& tag){
 if(key.size()!=32||nonce.size()!=12||input.size()>32768||(!encrypt&&tag.size()!=16))return {};
 auto ctx=std::unique_ptr<EVP_CIPHER_CTX,decltype(&EVP_CIPHER_CTX_free)>(EVP_CIPHER_CTX_new(),EVP_CIPHER_CTX_free);if(!ctx)return {};
 auto* c=ctx.get();int n=0,last=0;QByteArray out(input.size()+16,Qt::Uninitialized);
 if(EVP_CipherInit_ex(c,EVP_aes_256_gcm(),nullptr,nullptr,nullptr,encrypt?1:0)!=1||EVP_CIPHER_CTX_ctrl(c,EVP_CTRL_GCM_SET_IVLEN,nonce.size(),nullptr)!=1||EVP_CipherInit_ex(c,nullptr,nullptr,reinterpret_cast<const unsigned char*>(key.constData()),reinterpret_cast<const unsigned char*>(nonce.constData()),-1)!=1||EVP_CipherUpdate(c,nullptr,&n,reinterpret_cast<const unsigned char*>(aad.constData()),int(aad.size()))!=1||EVP_CipherUpdate(c,reinterpret_cast<unsigned char*>(out.data()),&n,reinterpret_cast<const unsigned char*>(input.constData()),int(input.size()))!=1)return {};
 if(!encrypt&&EVP_CIPHER_CTX_ctrl(c,EVP_CTRL_GCM_SET_TAG,16,tag.data())!=1)return {};
 if(EVP_CipherFinal_ex(c,reinterpret_cast<unsigned char*>(out.data())+n,&last)!=1)return {};
 out.resize(n+last);if(encrypt){tag.resize(16);if(EVP_CIPHER_CTX_ctrl(c,EVP_CTRL_GCM_GET_TAG,16,tag.data())!=1)return {};}
 return out;
}
inline QJsonObject seal(const QByteArray& key,const QString& room,const QString& sender,quint64 seq,const QJsonObject& body){
 const auto nonce=random(12);QByteArray tag;const auto aad=(room+":"+sender+":"+QString::number(seq)).toUtf8();
 const auto cipher=crypt(true,key,nonce,aad,QJsonDocument(body).toJson(QJsonDocument::Compact),tag);
 if(cipher.isEmpty())return {};return {{"sender",sender},{"seq",QString::number(seq)},{"box",QString::fromLatin1((nonce+tag+cipher).toBase64())}};
}
inline bool open(const QByteArray& key,const QString& room,const QJsonObject& envelope,QJsonObject& body){
 const auto sender=envelope["sender"].toString(),seq=envelope["seq"].toString();
 if(!QRegularExpression("^[a-f0-9]{32}$").match(sender).hasMatch()||!QRegularExpression("^[1-9][0-9]{0,8}$").match(seq).hasMatch())return false;
 const auto text=envelope["box"].toString();if(text.size()>44000)return false;
 const auto box=QByteArray::fromBase64(text.toLatin1(),QByteArray::AbortOnBase64DecodingErrors);if(box.size()<30||box.size()>32800)return false;
 auto tag=box.mid(12,16);const auto plain=crypt(false,key,box.first(12),(room+":"+sender+":"+seq).toUtf8(),box.mid(28),tag);
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(plain,&error);if(error.error!=QJsonParseError::NoError||!doc.isObject())return false;body=doc.object();return true;
}
}
