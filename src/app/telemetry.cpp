// Copyright (C) 2014-2024 Manuel Schneider

#include "app.h"
#include "extensionregistry.h"
#include "logging.h"
#include "pluginregistry.h"
#include "telemetry.h"
#include "telemetryprovider.h"
#include "usagedatabase.h"
#include "util.h"
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
static const char *CFG_TELEMETRY = "telemetry";
using namespace albert;


Telemetry::Telemetry(albert::ExtensionRegistry &registry) : registry_(registry)
{
    if (auto s = settings(); !s->contains(CFG_TELEMETRY))
    {
        auto text = tr(
            "Telemetry helps improving the user experience by collecing anonymous data. "
            "You can review the telemetry data to be sent in the settings. Do you want "
            "to enable telemetry? This configuration be changed at any time in the settings.");

        QMessageBox mb(QMessageBox::Question, qApp->applicationDisplayName(),
                       text, QMessageBox::No|QMessageBox::Yes);

        mb.setDefaultButton(QMessageBox::Yes);
        auto button = mb.exec();
        setEnabled(button == QMessageBox::Yes);
    }
    else
        enabled_ = s->value(CFG_TELEMETRY).toBool();
    last_report = state()->value(CFG_LAST_REPORT).toDateTime();

    QObject::connect(&timer, &QTimer::timeout, this, &Telemetry::trySendReport);
    timer.setInterval(60000);  // every minute
    if (enabled_)
        timer.start();
}

void Telemetry::trySendReport()
{
    auto now = QDateTime::currentDateTime();

    // At 3 AM most people are asleep. Use it as the beginning of a "human day".
    // Skip. Sent already today.
    if (now.addSecs(-10800).date() == last_report.addSecs(-10800).date())
        return;

    QString a = "Zffb,!!*\" $## $\"' **!";
    for (auto &c : a)
        c.unicode() = c.unicode() + 14;

    QNetworkRequest request((QUrl(a)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    DEBG << "trySendReport" << buildReportString();
    auto *reply = network()->put(request, buildReport().toJson(QJsonDocument::Compact));

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, now] {
        if (reply->error() != QNetworkReply::NoError)
        {
            WARN << "Failed to send telemetry data:";
            WARN << reply->errorString();
            auto json = QJsonDocument::fromJson(reply->readAll());
            WARN << json["error"].toString();
        }
        else
        {
            DEBG << "Successfully sent telemetry data.";
            state()->setValue(CFG_LAST_REPORT, last_report = now);
        }
        reply->deleteLater();
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

    data.insert("albert", albertTelemetry(last_report));

    if (auto *apps_plugin = registry_.extension<TelemetryProvider>("applications"); apps_plugin)
        data.insert("applications", apps_plugin->telemetryData());

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
        settings()->setValue(CFG_TELEMETRY, enabled_);
        enabled_ ? timer.start() : timer.stop();
    }
}
