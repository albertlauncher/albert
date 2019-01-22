// Copyright (C) 2014-2019 Manuel Schneider

#pragma once
#include <QJsonObject>
#include <QTimer>

namespace Core {

class Telemetry final
{
public:

    Telemetry();

    void enable(bool enable);
    bool isEnabled() const;
    QJsonObject buildReport();

private:

    void trySendReport();

    QTimer timer_;

};

}
