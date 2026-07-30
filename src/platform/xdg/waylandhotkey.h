// Copyright (c) 2026-2026 Aurelien Brabant

#pragma once
#include <QKeyCombination>
#include <QObject>
#include <expected>
#include <memory>

class WaylandHotkey : public QObject
{
    Q_OBJECT

public:
    ~WaylandHotkey();

    static bool isPlatformSupported();
    static std::expected<std::unique_ptr<WaylandHotkey>, QString> grab(QKeyCombination);

private:
    WaylandHotkey();

    class Private;
    std::unique_ptr<Private> d;

signals:
    void activated();
    void revoked(const QString &message);
};
