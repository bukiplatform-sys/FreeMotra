#pragma once
#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificate>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonObject>
#include <functional>
#include <memory>
#include <rtc/rtc.hpp>
#include "InternetSignal.h"
class LanServer;
class LanLink final : public QObject {
 Q_OBJECT
public:
 explicit LanLink(QObject* parent=nullptr);
 ~LanLink() override;
 QString listen(bool persistent=false, bool loopbackOnly=false);
 QString listenInternet(const QString& identity={},const QString& password={});
 bool internet()const{return internet_;}
 bool internetReady()const{return internet_&&cloud_.joined();}
 bool relayOnly=false; // Explicit diagnostics/testing; normal mode prefers direct ICE.
 void dial(const QString& invite,const QString& password,const QString& name);
 void approve(bool control, bool view=true, bool files=false, bool chat=false, bool voice=false);
 void stop();
 void send(const QByteArray& data);
 bool isHost() const { return host_; }
 bool controlAllowed() const { return control_; }
 bool viewAllowed() const { return view_; }
 bool filesAllowed() const { return files_; }
 bool chatAllowed() const { return chat_; }
 bool voiceAllowed() const { return voice_; }
 QString sessionBinding() const;
 bool active() const { return active_; }
 bool busy() const { return socket_ != nullptr || internet_; }
 std::function<bool(const QString&)> verifyPassword;
 static bool privateAddress(const QHostAddress& address);
 static QString address();
 quint64 queuedBytes() const;
 quint64 voiceQueuedBytes()const;
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
 InternetSignal cloud_;
 bool internet_=false;QString internetPeer_,dialPassword_,dialName_;
 std::unique_ptr<LanServer> server_;
 QSslSocket* socket_=nullptr;
 QByteArray buffer_, pin_;
 QSslCertificate cert_;
 QSslKey key_;
 QTimer deadline_, watchdog_;
 QElapsedTimer lastSeen_;
 std::shared_ptr<rtc::PeerConnection> peer_;
 std::shared_ptr<rtc::DataChannel> channel_,bulk_,voice_;
 quint64 epoch_=0;
 QList<QPair<QString,QString>> pendingCandidates_;
 bool remoteSdp_=false;
 QByteArray sessionNonce_;
 int signalingMessages_=0;
 int failures_=0;
 qint64 lockedUntil_=0;
 bool view_=false, files_=false, chat_=false, voice_=false;
 bool host_=false, authenticated_=false, approved_=false, active_=false, control_=false, stopping_=false;
 friend class LanServer;
};
