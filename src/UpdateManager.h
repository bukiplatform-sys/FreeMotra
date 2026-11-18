#pragma once
#include "UpdateRelease.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QNetworkReply>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QTimer>
#include <memory>
class UpdateManager:public QObject {
 Q_OBJECT
 Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
 Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY changed)
 Q_PROPERTY(QString notes READ notes NOTIFY changed)
 Q_PROPERTY(QString state READ state NOTIFY changed)
 Q_PROPERTY(bool available READ available NOTIFY changed)
 Q_PROPERTY(double progress READ progress NOTIFY changed)
 Q_PROPERTY(bool blocked READ blocked NOTIFY changed)
public:
 explicit UpdateManager(bool enabled=true,QObject* parent=nullptr);
 ~UpdateManager() override;
 QString currentVersion()const;
 QString latestVersion()const{return release_.version;}
 QString notes()const{return release_.notes;}
 QString state()const{return state_;}
 bool available()const{return fm::update::newer(release_.version,currentVersion());}
 double progress()const{return release_.size?double(received_)/double(release_.size):0;}
 bool blocked()const{return blocked_;}
 void setBlocked(bool value){if(blocked_!=value){blocked_=value;emit changed();}}
 Q_INVOKABLE void check();
 Q_INVOKABLE void download();
 Q_INVOKABLE void cancel();
 Q_INVOKABLE void install();
signals:
 void changed();
protected:
 virtual QNetworkReply* get(const QUrl& url,bool installer);
private:
 friend class UpdateManagerTests;
 void setState(const QString& value){state_=value;emit changed();}
 QNetworkAccessManager network_;
 QPointer<QNetworkReply> reply_;
 QTimer timer_;
 fm::update::Release release_;
 QString state_="idle",path_;
 bool blocked_=false,cancelled_=false,writeFailed_=false;
 qint64 received_=0;
 QByteArray metadata_;
 QCryptographicHash hash_{QCryptographicHash::Sha256};
 std::unique_ptr<QSaveFile> file_;
 std::unique_ptr<QTemporaryDir> directory_;
};
