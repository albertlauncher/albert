// SPDX-FileCopyrightText: 2024 Manuel Schneider

#pragma once
#include <QDebug>
#include <QString>
#include <chrono>
#include <albert/logging.h>

// Private API
namespace albert::detail
{

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
        CRIT << QStringLiteral(ccyan "%L1 µs | %2").arg(dur, 8).arg(name);
    }
};

}
