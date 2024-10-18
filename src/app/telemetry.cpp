// Copyright (C) 2014-2024 Manuel Schneider

#include "app.h"
#include "extensionregistry.h"
#include "albert.h"
#include "logging.h"
#include "pluginregistry.h"
#include "telemetry.h"
#include "telemetryprovider.h"
#include "usagedatabase.h"
#include <QCryptographicHash>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimeZone>
static const char *CFG_LAST_REPORT = "last_report";
static const char *CFG_TELEMETRY_ENABLED = "telemetry";
using namespace albert;


Telemetry::Telemetry(albert::ExtensionRegistry &registry):
    registry_(registry),
    last_report(state()->value(CFG_LAST_REPORT,  // Default to -24h avoid sending old data
                               QDateTime::currentDateTime().addDays(-1)).toDateTime())
{
    if (auto s = settings(); s->contains(CFG_TELEMETRY_ENABLED))
        enabled_ = s->value(CFG_TELEMETRY_ENABLED).toBool();
    else
    {
        auto text = tr(
            "Telemetry helps improving the user experience by collecing anonymous data. "
            "You can review the telemetry data to be sent in the settings. Do you want "
            "to enable telemetry? This configuration be changed at any time in the settings.");

        setEnabled(QMessageBox::question(0, qApp->applicationDisplayName(),
                                         text, QMessageBox::No|QMessageBox::Yes, QMessageBox::Yes)
                   == QMessageBox::Yes);
    }

    QObject::connect(&timer, &QTimer::timeout, this, &Telemetry::trySendReport);

    timer.start(60000);  // every minute
}

void Telemetry::trySendReport()
{
    auto now = QDateTime::currentDateTime();

    // Skip if sent already today.
    // At 3 AM most people are asleep. Use it as the beginning of a "human day".
    if (now.addSecs(-10800).date() == last_report.addSecs(-10800).date())
        return;

    QString a = "Zffb,!!*\" $## $\"' **!";
    for (auto &c : a)
        c.unicode() = c.unicode() + 14;

    QNetworkRequest request((QUrl(a)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    DEBG << "trySendReport" << buildReportString();
    auto *reply = network().put(request, buildReport().toJson(QJsonDocument::Compact));

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, now] {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError)
        {
            INFO << "Successfully sent telemetry data.";
            last_report = now;
            state()->setValue(CFG_LAST_REPORT, last_report);
        }
        else
        {
            WARN << "Failed to send telemetry data:";
            WARN << reply->errorString();
            auto json = QJsonDocument::fromJson(reply->readAll());
            WARN << json["error"].toString();
        }
    });
}

static QJsonObject albertTelemetry(const QDateTime &last_report)
{
    QJsonArray enabled_plugins;
    for (const auto &[id, plugin] : App::instance()->pluginRegistry().plugins())
        if (plugin.isEnabled())
            enabled_plugins.append(id);

    QJsonObject activationsSinceLastReport;
    for (const auto &[extension_id, activations] : UsageHistory::activationsSince(last_report))
        activationsSinceLastReport.insert(extension_id, (int)activations);

    QJsonObject o;
    o.insert("version", qApp->applicationVersion());
    o.insert("qt_version", qVersion());
    o.insert("kernel", QSysInfo::kernelType());
    o.insert("os", QSysInfo::prettyProductName());
    o.insert("os_type", QSysInfo::productType());
    o.insert("os_version", QSysInfo::productVersion());
    o.insert("platform", QGuiApplication::platformName());
    o.insert("enabled_plugins", enabled_plugins);
    o.insert("extension_activations", activationsSinceLastReport);

    return o;
}

static QString machineIdentifier()
{
    auto bytes = QSysInfo::machineUniqueId();
    bytes = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1);
    bytes = bytes.toHex();
    return QString::fromUtf8(bytes).left(8);
}

static QString iso8601now()
{
    auto now = QDateTime::currentDateTime();
    now.setTimeZone(QTimeZone::systemTimeZone());
    return now.toString(Qt::ISODate);
}

QJsonDocument Telemetry::buildReport() const
{
    QJsonObject data;

    if (enabled_)
    {
        data.insert("albert", albertTelemetry(last_report));
        if (auto *apps_plugin = registry_.extension<TelemetryProvider>("applications"); apps_plugin)
            data.insert("applications", apps_plugin->telemetryData());
    }

    QJsonObject o;
    o.insert("report", 2);  // report version
    o.insert("id", machineIdentifier());
    o.insert("time", iso8601now());
    o.insert("data", data);

    return QJsonDocument(o);
}

QString Telemetry::buildReportString() const
{
    return buildReport().toJson(QJsonDocument::Indented);
}

bool Telemetry::enabled() const { return enabled_; }

void Telemetry::setEnabled(bool value)
{
    if (enabled_ != value)
    {
        enabled_ = value;
        settings()->setValue(CFG_TELEMETRY_ENABLED, enabled_);
    }
}
