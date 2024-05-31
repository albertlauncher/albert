// Copyright (C) 2014-2024 Manuel Schneider

#pragma once
#include <QTimer>
class QJsonObject;

class Telemetry final
{
public:

    Telemetry();

    QJsonObject buildReport();
    QString buildReportString();

private:

    void trySendReport();

    QTimer timer;

};
