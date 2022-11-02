// Copyright (c) 2022 Manuel Schneider

#include <chrono>
#include "albert/logging.h"
#include <QString>
#include <utility>
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;
using time_point = std::chrono::time_point<system_clock>;


struct TimePrinter
{
    explicit TimePrinter(QString message) : msg(std::move(message)), begin_(system_clock::now()), end_()
    {

    }

    ~TimePrinter()
    {
        if (end_ == system_clock::time_point{})
            end();
    }

    void begin()
    {
        begin_ = system_clock::now();
    }

    void end()
    {
        end_ = system_clock::now();
        DEBG << qPrintable(msg.arg(duration_cast<microseconds>(end_-begin_).count(), 6));
    }
    void print(){ end_ = system_clock::now(); }

private:
    QString msg;
    time_point begin_;
    time_point end_;
};
