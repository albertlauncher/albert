// Copyright (C) 2014-2024 Manuel Schneider

#pragma once
#include <QCoreApplication>
#include <QDateTime>
#include <QObject>
#include <QTimer>
class QJsonDocument;
namespace albert {
class ExtensionRegistry;
}

class Telemetry final
{
    Q_DECLARE_TR_FUNCTIONS(Telemetry)

public:
    Telemetry(albert::ExtensionRegistry &registry);

    QJsonDocument buildReport() const;
    QString buildReportString() const;

    bool enabled() const;
    void setEnabled(bool);

private:
    void trySendReport();

    albert::ExtensionRegistry &registry_;
    QTimer timer;
    QDateTime last_report;
    bool enabled_;
};
