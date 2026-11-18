#pragma once
#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificate>
#include <QTimer>
#include <QJsonObject>
#include <functional>
#include <memory>
#include <rtc/rtc.hpp>
class LanServer;
class LanLink final : public QObject {
 Q_OBJECT
public:
 explicit LanLink(QObject* parent=nullptr);
 ~LanLink() override;
 QString listen();
 void dial(const QString& invite,const QString& password,const QString& name);
 void approve(bool control);
 void stop();
 void send(const QByteArray& data);
 bool isHost() const { return host_; }
 bool controlAllowed() const { return control_; }
 QString sessionBinding() const;
 bool active() const { return active_; }
 std::function<bool(const QString&)> verifyPassword;
 static bool privateAddress(const QHostAddress& address);
 static QString address();
 quint64 queuedBytes() const;
 signals:
 void status(const QString& text);
 void request(const QString& peer);
 void opened();
 void ended();
 void message(const QByteArray& bytes);
 private:
 void incoming(qintptr fd);
 void attach(QSslSocket* socket);
 void read();
 void signal(const QJsonObject& value);
 void received(const QJsonObject& value);
 void createPeer(bool offer);
 void channel(std::shared_ptr<rtc::DataChannel> value);
 void fail(const QString& reason);
 std::unique_ptr<LanServer> server_;
 QSslSocket* socket_=nullptr;
 QByteArray buffer_, pin_;
 QSslCertificate cert_;
 QSslKey key_;
 QTimer deadline_, watchdog_;
 std::shared_ptr<rtc::PeerConnection> peer_;
 std::shared_ptr<rtc::DataChannel> channel_;
 quint64 epoch_=0;
 int failures_=0;
 qint64 lockedUntil_=0;
 bool host_=false, authenticated_=false, approved_=false, active_=false, control_=false, stopping_=false;
 friend class LanServer;
};
