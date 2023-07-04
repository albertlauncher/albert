// Copyright (c) 2023 Manuel Schneider

#include "albert/logging.h"
#include "albert/util/timeprinter.h"
using namespace albert;
using namespace std;

TimePrinter::TimePrinter(QString message) : msg(::move(message))
{
    restart();
}

TimePrinter::~TimePrinter()
{
    stop();
}

void TimePrinter::restart()
{
    begin_ = chrono::system_clock::now();
    end_ = chrono::system_clock::time_point{};
}

void TimePrinter::stop()
{
    if (end_ == chrono::system_clock::time_point{})
        end_ = chrono::system_clock::now();
    DEBG << qPrintable(msg.arg(chrono::duration_cast<chrono::microseconds>(end_-begin_).count(), 6));
}
