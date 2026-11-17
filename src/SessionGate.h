#pragma once
#include <QString>
namespace fm {
// No network endpoint is exposed by this foundation. A future authenticated
// transport must own this object. Neither QML nor remote input can grant consent.
class SessionGate {
public:
    enum class State { Idle, AwaitingConsent, Viewing, Controlling };
    bool authenticatedPeer(const QString& fingerprint);
    bool localConsent(bool allowControl);
    bool mayView() const;
    bool mayControl() const;
    void end();
    State state() const { return state_; }
    QString peer() const { return peer_; }
private:
    State state_ = State::Idle;
    QString peer_;
};
}
