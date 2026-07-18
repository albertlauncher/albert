// Copyright (C) 2026-2026 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
class Hotkey;
class QSettings;

class HotkeyManager : public QObject
{
    Q_OBJECT
public:
    HotkeyManager(const QSettings &settings);
    ~HotkeyManager();

    Hotkey *hotkey() const;
    void setHotkey(std::unique_ptr<Hotkey> hotkey);

private:
    std::unique_ptr<Hotkey> hotkey_;

signals:
    void activated();

};
