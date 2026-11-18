#pragma once
#include "../SessionGate.h"
#include <QSet>
#include <QString>
#include <windows.h>
namespace fm {
// To be owned by the authenticated host session, never exposed directly to QML.
class InputController {
public:
    explicit InputController(const SessionGate& gate) : gate_(gate) {}
    ~InputController() { releaseAll(); }
    bool move(double x, double y);
    bool key(unsigned short virtualKey, bool down);
    bool text(const QString& text);
    bool button(bool right, bool down);
    bool wheel(int delta);
    void releaseAll();
private:
    const SessionGate& gate_;
    QSet<unsigned short> keys_;
    bool left_ = false, right_ = false;
};
}
