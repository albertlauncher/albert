// SPDX-FileCopyrightText: 2025 Manuel Schneider

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
    QString name_;
    std::chrono::system_clock::time_point start_;

    [[nodiscard]] TimeIt(const QString &name = {}):
        name_(name),
        start_(std::chrono::system_clock::now())
    {}

    ~TimeIt()
    {
        auto end = std::chrono::system_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        CRIT << QStringLiteral(ccyan "%L1 µs | %2").arg(dur, 8).arg(name_);
    }
};

}
