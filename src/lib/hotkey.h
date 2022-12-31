#pragma once
#include <QKeyCombination>
#include <memory>
class QHotkey;

class Hotkey
{
public:
    Hotkey();

    QKeyCombination hotkey() const;
    bool setHotkey(QKeyCombination);

    static bool isPlatformSupported();

private:
    std::unique_ptr<QHotkey> hotkey_;
};
