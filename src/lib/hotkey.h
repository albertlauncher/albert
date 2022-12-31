#pragma once
#include <memory>
class QHotkey;

class Hotkey
{
public:
    Hotkey();

    int hotkey() const;
    bool setHotkey(int);

    static bool isPlatformSupported();

private:
    std::unique_ptr<QHotkey> hotkey_;
};
