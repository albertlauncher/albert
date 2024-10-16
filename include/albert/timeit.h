// SPDX-FileCopyrightText: 2024 Manuel Schneider

#pragma once
#include <QDebug>
#include <QString>
#include <chrono>

// Private API

struct TimeIt
{
    QString name;
    std::chrono::system_clock::time_point start;

    [[nodiscard]] TimeIt(const QString &name = {}):
        name(name),
        start(std::chrono::system_clock::now())
    {}

    ~TimeIt()
    {
        auto end = std::chrono::system_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        qCritical() << QString("%L1 µs | %2").arg(dur, 8).arg(name);
    }
};
