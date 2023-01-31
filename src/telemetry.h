// Copyright (C) 2014-2023 Manuel Schneider

#pragma once
#include <QJsonObject>
#include <QTimer>

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
