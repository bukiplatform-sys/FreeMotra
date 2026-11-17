#pragma once
#include "Identity.h"
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
public:
    explicit AppController(QObject* parent = nullptr);
    QString deviceId() const { return identity_.deviceId(); }
    QString password() const { return password_; }
    QString language() const { return settings_.value("ui/language", "tr").toString(); }
    bool dark() const { return settings_.value("ui/dark", true).toBool(); }
    QString quality() const { return settings_.value("video/quality", "balanced").toString(); }
    bool notifications() const { return settings_.value("ui/notifications", true).toBool(); }
    QString message() const { return message_; }
    void setLanguage(const QString& value);
    void setDark(bool value);
    void setQuality(const QString& value);
    void setNotifications(bool value);
    Q_INVOKABLE void refreshPassword();
    Q_INVOKABLE void copyId();
    Q_INVOKABLE void requestConnection(const QString& remote);
    Q_INVOKABLE void dismissMessage();
signals:
    void passwordChanged();
    void settingsChanged();
    void messageChanged();
private:
    QSettings settings_;
    fm::Identity identity_;
    QString password_, message_;
    QTimer expiryTimer_;
};
