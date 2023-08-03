// Copyright (c) 2023 Manuel Schneider

#pragma once
#include <QObject>
#include <QKeyCombination>
#include <memory>
class QHotkey;

class Hotkey : public QObject
{
    Q_OBJECT

public:
    Hotkey();
    ~Hotkey();

    QKeyCombination hotkey() const;
    bool setHotkey(QKeyCombination);

    static bool isPlatformSupported();

private:
    std::unique_ptr<QHotkey> hotkey_;

signals:
    void activated();
};
