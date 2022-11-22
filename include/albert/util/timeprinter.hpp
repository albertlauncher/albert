// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <chrono>
#include "albert/logging.h"
#include <QString>
#include <utility>

namespace albert {

struct TimePrinter
{
    explicit TimePrinter(QString message) : msg(std::move(message)),
                                            begin_(std::chrono::system_clock::now()), end_()
    {

    }

    ~TimePrinter()
    {
        if (end_ == std::chrono::system_clock::time_point{})
            end();
    }

    void begin()
    {
        begin_ = std::chrono::system_clock::now();
    }

    void end()
    {
        end_ = std::chrono::system_clock::now();
        DEBG << qPrintable(msg.arg(std::chrono::duration_cast<std::chrono::microseconds>(end_-begin_).count(), 6));
    }
    void print(){ end_ = std::chrono::system_clock::now(); }

private:
    QString msg;
    std::chrono::time_point<std::chrono::system_clock> begin_;
    std::chrono::time_point<std::chrono::system_clock> end_;
};

}
