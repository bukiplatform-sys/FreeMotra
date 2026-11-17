#pragma once
#include <QByteArray>
#include <QDateTime>
#include <QSettings>
#include <QString>
#include <memory>

namespace fm {
QByteArray secureBytes(int count);
class Identity {
public:
    explicit Identity(QSettings& settings);
    QString deviceId() const { return id_; }
    QString refreshPassword();
    bool verifyAndConsume(const QString& candidate, QDateTime now = QDateTime::currentDateTimeUtc());
    bool locked(QDateTime now = QDateTime::currentDateTimeUtc()) const;
    QDateTime expires() const { return expires_; }
private:
    QString id_;
    QByteArray salt_, verifier_;
    QDateTime expires_, lockedUntil_;
    int failures_ = 0;
    bool consumed_ = true;
};
}
