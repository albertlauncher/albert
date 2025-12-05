// Copyright (C) 2014-2024 Manuel Schneider

#pragma once
#include <QDateTime>
#include <QObject>
#include <QTimer>
class PluginRegistry;
class QJsonDocument;
class QJsonObject;
namespace albert { class ExtensionRegistry; }

class Telemetry : public QObject
{
    Q_OBJECT

public:
    Telemetry(PluginRegistry &, albert::ExtensionRegistry &);

    QJsonDocument buildReport() const;
    QString buildReportString() const;

    bool enabled() const;
    void setEnabled(bool);

private:
    void trySendReport();
    QJsonObject albertTelemetry() const;

    PluginRegistry &plugin_registry_;
    albert::ExtensionRegistry &extension_registry_;
    QTimer timer;
    QDateTime last_report;
    bool enabled_;
};
