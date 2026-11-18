#pragma once
#include <QJsonObject>
#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QHash>
class InternetSignal:public QObject {
 Q_OBJECT
public:
 explicit InternetSignal(QObject* parent=nullptr);
 ~InternetSignal()override{socket_.disconnect(this);stop();}
 QString host();bool join(const QString& invite);
 bool send(const QJsonObject& body);
 void stop();bool joined()const{return joined_;}
 QString binding()const;
signals:
 void ready();void message(const QString& sender,const QJsonObject& value);void failed();
private:
 void connectServer();void frame(const QString& event,const QJsonObject& payload,const QString& topic={});
 QWebSocket socket_;QTimer heartbeat_,deadline_;
 QString room_,sender_,topic_;QByteArray key_;QHash<QString,quint64> seen_;
 quint64 counter_=0,ref_=1;bool joined_=false,stopping_=false;
};
