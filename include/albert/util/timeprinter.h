// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include "albert/logging.h"
#include <QString>
#include <chrono>

namespace albert {

template <class UNIT = std::chrono::milliseconds>
class ALBERT_EXPORT TimePrinter
{
public:

    using time_point = std::chrono::system_clock::time_point;

    explicit TimePrinter(QString msg) : message(std::move(msg))
    {
        restart();
    }

    TimePrinter(const TimePrinter&other)
        :  message(other.message), begin_(other.begin_), end_(other.end_)
    { }

    TimePrinter(TimePrinter&&other)
        :  message(std::move(other.message)), begin_(std::move(other.begin_)), end_(std::move(other.end_))
    { }

    TimePrinter &operator=(const TimePrinter &other)
    {
        stop();
        message = other.message;
        begin_ = other.begin_;
        end_ = other.end_;
        return *this;
    }

    TimePrinter &operator=(TimePrinter &&other)
    {
        stop();
        message = std::move(other.message);
        begin_ = std::move(other.begin_);
        end_ = std::move(other.end_);
        return *this;
    }

    ~TimePrinter()
    {
        stop();
    }

    void restart(QString msg = {})
    {
        stop();
        if (!msg.isNull())
            message = std::move(msg);
        begin_ = std::chrono::system_clock::now();
        end_ = time_point{};
    }

    void stop()
    {
        if (begin_ != time_point{} && end_ == time_point{}){
            end_ = std::chrono::system_clock::now();
            DEBG << qPrintable(message.arg(std::chrono::duration_cast<UNIT>(end_-begin_).count(), 6));
        }
    }

    QString message;

private:
    time_point begin_;
    time_point end_;
};

}
