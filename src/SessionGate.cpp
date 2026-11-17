#include "SessionGate.h"
#include <QRegularExpression>
namespace fm {
bool SessionGate::authenticatedPeer(const QString& fingerprint) {
    if (state_ != State::Idle || !QRegularExpression("^[0-9a-fA-F]{64}$").match(fingerprint).hasMatch()) return false;
    peer_ = fingerprint.toLower();
    state_ = State::AwaitingConsent;
    return true;
}
bool SessionGate::localConsent(bool allowControl) {
    if (state_ != State::AwaitingConsent) return false;
    state_ = allowControl ? State::Controlling : State::Viewing;
    return true;
}
bool SessionGate::mayView() const { return state_ == State::Viewing || state_ == State::Controlling; }
bool SessionGate::mayControl() const { return state_ == State::Controlling; }
void SessionGate::end() { state_ = State::Idle; peer_.clear(); }
}
