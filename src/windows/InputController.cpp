#include "InputController.h"
#include <algorithm>
#include <cmath>
namespace fm {
bool InputController::move(double x, double y) {
    if (!gate_.mayControl() || !std::isfinite(x) || !std::isfinite(y)) return false;
    INPUT i{}; i.type = INPUT_MOUSE;
    i.mi.dx = LONG(std::clamp(x, 0.0, 1.0) * 65535); i.mi.dy = LONG(std::clamp(y, 0.0, 1.0) * 65535);
    i.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    return SendInput(1, &i, sizeof(i)) == 1;
}
bool InputController::key(unsigned short vk, bool down) {
    if (!gate_.mayControl() || vk < 8 || vk > 254 || (!down && !keys_.contains(vk))) return false;
    if (down && !keys_.contains(vk)) keys_.insert(vk); // Retain even on uncertain SendInput result for cleanup.
    INPUT i{}; i.type = INPUT_KEYBOARD; i.ki.wVk = vk; i.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
    const bool ok = SendInput(1, &i, sizeof(i)) == 1;
    if (ok && !down) keys_.remove(vk);
    return ok;
}
bool InputController::button(bool right, bool down) {
    if (!gate_.mayControl()) return false;
    auto& held = right ? right_ : left_;
    if (!down && !held) return false;
    if (down) held = true;
    INPUT i{}; i.type = INPUT_MOUSE;
    i.mi.dwFlags = right ? (down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP) : (down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP);
    const bool ok = SendInput(1, &i, sizeof(i)) == 1;
    if (ok && !down) held = false;
    return ok;
}
bool InputController::wheel(int delta) {
    if (!gate_.mayControl() || delta < -1200 || delta > 1200) return false;
    INPUT i{}; i.type = INPUT_MOUSE; i.mi.dwFlags = MOUSEEVENTF_WHEEL; i.mi.mouseData = DWORD(delta);
    return SendInput(1, &i, sizeof(i)) == 1;
}
void InputController::releaseAll() {
    // Cleanup must run even after authorization has been revoked.
    for (auto vk : keys_) {
        INPUT i{}; i.type = INPUT_KEYBOARD; i.ki.wVk = vk; i.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &i, sizeof(i));
    }
    keys_.clear();
    if (left_ || right_) {
        INPUT i{}; i.type = INPUT_MOUSE;
        i.mi.dwFlags = (left_ ? MOUSEEVENTF_LEFTUP : 0) | (right_ ? MOUSEEVENTF_RIGHTUP : 0);
        SendInput(1, &i, sizeof(i));
    }
    left_ = right_ = false;
}
}
