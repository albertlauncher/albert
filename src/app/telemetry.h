// Copyright (C) 2014-2024 Manuel Schneider

#pragma once
#include <QDateTime>
#include <QObject>
#include <QTimer>
class QJsonDocument;
namespace albert {
class ExtensionRegistry;
}

class Telemetry : public QObject
{
    Q_OBJECT

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
