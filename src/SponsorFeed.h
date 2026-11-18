#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QVariantMap>
class SponsorFeed final:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantMap current READ current NOTIFY changed)
 Q_PROPERTY(QString image READ image NOTIFY changed)
 Q_PROPERTY(bool sample READ sample NOTIFY changed)
 Q_PROPERTY(bool available READ available NOTIFY changed)
public:
 explicit SponsorFeed(QObject* parent=nullptr);
 QVariantMap current()const{return current_;}
 QString image()const{return image_;}
 bool sample()const{return sample_;}
 bool available()const{return available_;}
 void pause(bool value);
 Q_INVOKABLE void openLink();
 signals:void changed();
private:
 void refresh();
 void fetch(const QUrl& url,bool image);
 QNetworkAccessManager network_;
 QNetworkReply* reply_=nullptr;
 QTimer timer_;
 QUrl feed_;
 QVariantMap current_;
 QString image_;
 bool paused_=false,available_=false,sample_=false;
};
