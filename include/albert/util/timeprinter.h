// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <chrono>
#include <QString>

namespace albert {

struct ALBERT_EXPORT TimePrinter
{
    explicit TimePrinter(QString message);
    ~TimePrinter();
    void restart();
    void stop();

private:
    QString msg;
    std::chrono::time_point<std::chrono::system_clock> begin_;
    std::chrono::time_point<std::chrono::system_clock> end_;
};

}
