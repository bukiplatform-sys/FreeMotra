#pragma once
#include "Identity.h"
#include "LanLink.h"
#include "SessionGate.h"
#include "SessionTools.h"
#include "AdaptiveQuality.h"
#include <QVariantList>
#include "windows/DesktopCapture.h"
#include "windows/InputController.h"
#include <QImage>
#include <QElapsedTimer>
#include <QObject>
#include <QSettings>
#include <QTimer>
class AppController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString deviceId READ deviceId CONSTANT)
    Q_PROPERTY(QString password READ password NOTIFY passwordChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY settingsChanged)
    Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY settingsChanged)
    Q_PROPERTY(QString quality READ quality WRITE setQuality NOTIFY settingsChanged)
    Q_PROPERTY(bool notifications READ notifications WRITE setNotifications NOTIFY settingsChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(QString invitation READ invitation NOTIFY sessionChanged)
    Q_PROPERTY(QString sessionStatus READ sessionStatus NOTIFY sessionChanged)
    Q_PROPERTY(QString incomingPeer READ incomingPeer NOTIFY sessionChanged)
    Q_PROPERTY(bool sessionActive READ sessionActive NOTIFY sessionChanged)
    Q_PROPERTY(bool hosting READ hosting NOTIFY sessionChanged)
    Q_PROPERTY(bool remoteControl READ remoteControl NOTIFY sessionChanged)
    Q_PROPERTY(QVariantList savedDevices READ savedDevices NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList monitors READ monitors NOTIFY sessionChanged)
    Q_PROPERTY(int selectedMonitor READ selectedMonitor NOTIFY sessionChanged)
    Q_PROPERTY(QString effectiveQuality READ effectiveQuality NOTIFY sessionChanged)
    Q_PROPERTY(bool viewAllowed READ viewAllowed NOTIFY sessionChanged)
    Q_PROPERTY(int frameSerial READ frameSerial NOTIFY frameChanged)
public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;
    QString invitation() const { return invitation_; }
    QString sessionStatus() const { return sessionStatus_; }
    QString incomingPeer() const { return incomingPeer_; }
    bool sessionActive() const { return link_.active(); }
    bool hosting() const { return link_.isHost(); }
    bool remoteControl() const { return link_.controlAllowed(); }
    int frameSerial() const { return frameSerial_; }
    Q_INVOKABLE void startHosting();
    Q_INVOKABLE void connectLan(const QString& invite,const QString& password);
    Q_INVOKABLE void approve(bool control);
    Q_INVOKABLE void approvePermissions(bool view,bool control,bool files,bool chat,bool voice);
    Q_INVOKABLE void saveDevice(const QString& name,const QString& invitation);
    Q_INVOKABLE void removeDevice(int index);
    Q_INVOKABLE void toggleFavorite(int index);
    Q_INVOKABLE void selectMonitor(int index);
    QVariantList savedDevices() const { return settings_.value("devices/bookmarks").toList(); }
    QVariantList monitors() const { return monitors_; }
    int selectedMonitor() const { return monitorIndex_; }
    QString effectiveQuality() const { return effectiveQuality_; }
    bool viewAllowed() const { return link_.viewAllowed(); }
    SessionTools* tools() { return &tools_; }
    Q_INVOKABLE void endSession();
    Q_INVOKABLE void copyInvitation();
    Q_INVOKABLE void remoteInput(const QString& kind,double a,double b=0);
    Q_INVOKABLE void releaseRemoteKeys();
    QImage frame() const { return frame_; }
    QString deviceId() const { return identity_.deviceId(); }
    QString password() const { return password_; }
    QString language() const { return settings_.value("ui/language", "tr").toString(); }
    bool dark() const { return settings_.value("ui/dark", true).toBool(); }
    QString quality() const { return settings_.value("video/quality", "auto").toString(); }
    bool notifications() const { return settings_.value("ui/notifications", true).toBool(); }
    QString message() const { return message_; }
    void setLanguage(const QString& value);
    void setDark(bool value);
    void setQuality(const QString& value);
    void setNotifications(bool value);
    Q_INVOKABLE void refreshPassword();
    Q_INVOKABLE void copyId();
    Q_INVOKABLE void dismissMessage();
signals:
    void devicesChanged();
    void sessionChanged();
    void frameChanged();
    void passwordChanged();
    void settingsChanged();
    void messageChanged();
private:
    void captureTick();
    void receive(const QByteArray& bytes);
    void input(const QJsonObject& value);
    LanLink link_;
    SessionTools tools_{link_};
    QVariantList monitors_;
    int monitorIndex_=0, displayedMonitor_=0, incomingMonitor_=0;
    fm::AdaptiveQuality adaptive_;
    QString effectiveQuality_="Otomatik / Auto";
    fm::SessionGate gate_;
    fm::InputController input_{gate_};
    fm::DesktopCapture capture_;
    QTimer captureTimer_;
    QElapsedTimer frameWait_;
    bool waitingFrame_=false;
    QString invitation_, sessionStatus_, incomingPeer_;
    QImage frame_;
    QByteArray frameBytes_;
    int expectedBytes_=0, frameSerial_=0;
    QSettings settings_;
    fm::Identity identity_;
    QString password_, message_;
    QTimer expiryTimer_;
};
