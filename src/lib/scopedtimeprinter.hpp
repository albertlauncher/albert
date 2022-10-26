// Copyright (c) 2022 Manuel Schneider

#include <chrono>
#include "albert/logging.h"
#include <QString>

class  ScopedTimePrinter
{
public:
    ScopedTimePrinter(const QString &message)
        : msg(message), ctime(std::chrono::system_clock::now())
    {

    }

    ~ScopedTimePrinter()
    {
        DEBG << qPrintable(msg.arg(
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now()-ctime).count(), 6
        ));
    }

    QString msg;
    std::chrono::system_clock::time_point ctime;
};

