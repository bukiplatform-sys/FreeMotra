#include "InternetSignal.h"
#include "SignalCrypto.h"
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QNetworkRequest>
namespace {
// Public anon token; database access remains restricted by RLS. No administrator or service-role credentials.
const QString key=QStringLiteral("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im1lc3FxeGxhY2p1dmluc2Rvc3B5Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3OTQ5OTc4MDksImV4cCI6MjExMDU3MzgwOX0.hY0tb4iebC_81Gr1QHvgW4TSe7o9x-EuB7-IM8xc_lA");
}
InternetSignal::InternetSignal(QObject* parent):QObject(parent) {
 socket_.setMaxAllowedIncomingMessageSize(65536);socket_.setMaxAllowedIncomingFrameSize(65536);
 heartbeat_.setInterval(25000);deadline_.setSingleShot(true);
 connect(&heartbeat_,&QTimer::timeout,this,[this]{frame("heartbeat",{},"phoenix");});
 connect(&deadline_,&QTimer::timeout,this,[this]{stop();emit failed();});
 connect(&socket_,&QWebSocket::connected,this,[this]{
  frame("phx_join",{{"access_token",key},{"config",QJsonObject{{"broadcast",QJsonObject{{"ack",false},{"self",false}}},{"presence",QJsonObject{{"enabled",false}}},{"private",false}}}});
 });
 connect(&socket_,&QWebSocket::disconnected,this,[this]{if(!stopping_&&!room_.isEmpty()){stop();emit failed();}});
 connect(&socket_,&QWebSocket::textMessageReceived,this,[this](const QString& text){
  if(text.size()>65536){stop();emit failed();return;}
  const auto o=QJsonDocument::fromJson(text.toUtf8()).object();if(o["topic"].toString()!=topic_)return;
  const auto event=o["event"].toString();const auto payload=o["payload"].toObject();
  if(event=="phx_reply"&&o["ref"].toString()=="1"){
   if(payload["status"].toString()!="ok"){stop();emit failed();return;}joined_=true;deadline_.stop();heartbeat_.start();emit ready();return;
  }
  if(event=="phx_error"||event=="phx_close"){stop();emit failed();return;}
  if(event!="broadcast"||payload["event"].toString()!="signal"||!joined_)return;
  const auto envelope=payload["payload"].toObject();const auto sender=envelope["sender"].toString();if(sender==sender_)return;
  QJsonObject value;if(!fm::signal::open(key_,room_,envelope,value))return;
  const auto seq=envelope["seq"].toString().toULongLong();if(seq<=seen_.value(sender)||seen_.size()>=16&&!seen_.contains(sender))return;
  seen_[sender]=seq;emit message(sender,value);
 });
}
QString InternetSignal::host(){stop();room_=QString::fromLatin1(fm::signal::random(16).toHex());key_=fm::signal::random(32);if(room_.size()!=32||key_.size()!=32)return {};const auto invite="fm3://"+room_+"/"+QString::fromLatin1(key_.toHex());connectServer();return invite;}
bool InternetSignal::join(const QString& invite){stop();if(!fm::signal::invitation(invite,room_,key_))return false;connectServer();return true;}
void InternetSignal::connectServer(){
 sender_=QString::fromLatin1(fm::signal::random(16).toHex());if(sender_.size()!=32){stop();emit failed();return;}topic_="realtime:freemotra:"+room_;counter_=0;ref_=1;seen_.clear();
 QUrl url("wss://mesqqxlacjuvinsdospy.supabase.co/realtime/v1/websocket");QUrlQuery q;q.addQueryItem("apikey",key);q.addQueryItem("vsn","1.0.0");url.setQuery(q);
 QNetworkRequest request(url);request.setTransferTimeout(15000);socket_.open(request);deadline_.start(20000);
}
void InternetSignal::frame(const QString& event,const QJsonObject& payload,const QString& topic){socket_.sendTextMessage(QString::fromUtf8(QJsonDocument(QJsonObject{{"topic",topic.isEmpty()?topic_:topic},{"event",event},{"payload",payload},{"ref",QString::number(ref_++)},{"join_ref","1"}}).toJson(QJsonDocument::Compact)));}
bool InternetSignal::send(const QJsonObject& value){if(!joined_||counter_>=256||socket_.bytesToWrite()>131072)return false;const auto payload=fm::signal::seal(key_,room_,sender_,++counter_,value);if(payload.isEmpty())return false;frame("broadcast",{{"type","broadcast"},{"event","signal"},{"payload",payload}});return true;}
QString InternetSignal::binding()const{return room_.isEmpty()?QString():QString::fromLatin1(QCryptographicHash::hash(room_.toUtf8()+key_,QCryptographicHash::Sha256).toHex());}
void InternetSignal::stop(){stopping_=true;heartbeat_.stop();deadline_.stop();joined_=false;room_.clear();topic_.clear();sender_.clear();key_.fill('\0');key_.clear();seen_.clear();socket_.abort();stopping_=false;}
