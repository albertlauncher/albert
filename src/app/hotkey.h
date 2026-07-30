// Copyright (C) 2026-2026 Manuel Schneider

#pragma once
#include <QKeyCombination>
#include <QObject>
#include <expected>
#include <memory>

class Hotkey : public QObject
{
    Q_OBJECT

public:
    Hotkey(QKeyCombination);
    ~Hotkey();

    QKeyCombination keyCombination() const;
    QString portableString() const;
    QString nativeString() const;

    static bool isPlatformSupported();
    static std::expected<std::unique_ptr<Hotkey>, QString> grab(QKeyCombination);
    static std::expected<std::unique_ptr<Hotkey>, QString> grab(const QString &);


private:
    class Private;
    std::unique_ptr<Private> d;

signals:
    void activated();
    void revoked(const QString &message);
};
