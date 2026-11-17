#include "AppController.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QRegularExpression>
AppController::AppController(QObject* parent) : QObject(parent), identity_(settings_) {
    refreshPassword();
    expiryTimer_.setInterval(300000);
    connect(&expiryTimer_, &QTimer::timeout, this, &AppController::refreshPassword);
    expiryTimer_.start();
}
void AppController::refreshPassword() {
    password_.fill(QChar(0));
    password_ = identity_.refreshPassword();
    emit passwordChanged();
    if (expiryTimer_.isActive()) expiryTimer_.start();
}
void AppController::setLanguage(const QString& v) {
    if (v != "tr" && v != "en") return;
    settings_.setValue("ui/language", v); emit settingsChanged();
}
void AppController::setDark(bool v) { settings_.setValue("ui/dark", v); emit settingsChanged(); }
void AppController::setQuality(const QString& v) {
    if (v != "balanced" && v != "quality" && v != "speed") return;
    settings_.setValue("video/quality", v); emit settingsChanged();
}
void AppController::setNotifications(bool v) { settings_.setValue("ui/notifications", v); emit settingsChanged(); }
void AppController::copyId() { QGuiApplication::clipboard()->setText(deviceId()); }
void AppController::requestConnection(const QString& remote) {
    const bool tr = language() == "tr";
    if (!QRegularExpression("^FM-[0-9A-Fa-f]{32}$").match(remote.trimmed()).hasMatch())
        message_ = tr ? "Geçerli bir FM- cihaz kimliği girin." : "Enter a valid FM- device identity.";
    else if (remote.trimmed().compare(deviceId(), Qt::CaseInsensitive) == 0)
        message_ = tr ? "Bu cihazın kendisine bağlanamazsınız." : "You cannot connect to this device itself.";
    else
        message_ = tr ? "Bağlantı motoru bu sürümde henüz hazır değil. Ağ bağlantısı başlatılmadı." : "The connection engine is not available in this version. No network connection was started.";
    emit messageChanged();
}
void AppController::dismissMessage() { message_.clear(); emit messageChanged(); }
