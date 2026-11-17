#include "Identity.h"
#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QRegularExpression>
#include <windows.h>
#include <bcrypt.h>
#include <stdexcept>

namespace fm {
QByteArray secureBytes(int count) {
    if (count < 1 || count > 4096) throw std::invalid_argument("random byte count");
    QByteArray bytes(count, Qt::Uninitialized);
    if (BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(bytes.data()), ULONG(count), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
        throw std::runtime_error("Windows secure random generator failed");
    return bytes;
}
Identity::Identity(QSettings& settings) {
    id_ = settings.value("identity/deviceId").toString();
    if (!QRegularExpression("^FM-[0-9A-F]{32}$").match(id_).hasMatch()) {
        id_ = "FM-" + QString::fromLatin1(secureBytes(16).toHex().toUpper());
        settings.setValue("identity/deviceId", id_);
        settings.sync();
        if (settings.status() != QSettings::NoError) throw std::runtime_error("Cannot persist device identity");
    }
}
QString Identity::refreshPassword() {
    // 32 symbols, rejection-free mapping from independent random bytes: 60 bits.
    constexpr auto alphabet = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QString password;
    for (unsigned char c : secureBytes(12)) password += QLatin1Char(alphabet[c & 31]);
    salt_ = secureBytes(32);
    verifier_ = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), salt_, 120000, 32);
    expires_ = QDateTime::currentDateTimeUtc().addSecs(300);
    consumed_ = false;
    // Rotation deliberately does not reset the brute-force lockout.
    return password;
}
bool Identity::locked(QDateTime now) const { return now < lockedUntil_; }
bool Identity::verifyAndConsume(const QString& candidate, QDateTime now) {
    if (locked(now) || consumed_ || now >= expires_ || candidate.size() > 128) return false;
    auto hash = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, candidate.toUtf8(), salt_, 120000, 32);
    unsigned int difference = unsigned(hash.size() ^ verifier_.size());
    for (qsizetype i = 0; i < hash.size() && i < verifier_.size(); ++i)
        difference |= static_cast<unsigned char>(hash[i]) ^ static_cast<unsigned char>(verifier_[i]);
    if (difference != 0) {
        if (++failures_ >= 5) { lockedUntil_ = now.addSecs(60); failures_ = 0; }
        return false;
    }
    consumed_ = true;
    failures_ = 0;
    verifier_.fill('\0');
    verifier_.clear();
    return true;
}
}
