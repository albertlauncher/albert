// Copyright (C) 2014-2025 Manuel Schneider

#include "app.h"
#include "extensionregistry.h"
#include "logging.h"
#include "networkutil.h"
#include "pluginregistry.h"
#include "telemetry.h"
#include "telemetryprovider.h"
#include "usagedatabase.h"
#include <QCoroNetworkReply>
#include <QCoroTask>
#include <QCryptographicHash>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRestReply>
#include <QSettings>
#include <QTimeZone>
#include <expected>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

namespace
{
const auto *K_LAST_REPORT = "last_report";
const auto *K_TELEMETRY_ENABLED = "telemetry";
}


Telemetry::Telemetry(PluginRegistry &pr, ExtensionRegistry &er):
    plugin_registry_(pr),
    extension_registry_(er),
    last_report(App::state()->value(K_LAST_REPORT,  // Default to -24h avoid sending old data
                                    QDateTime::currentDateTime().addDays(-1)).toDateTime())
{
    if (auto s = App::settings(); s->contains(K_TELEMETRY_ENABLED))
        enabled_ = s->value(K_TELEMETRY_ENABLED).toBool();
    else
    {
        auto text = tr(
            "Albert collects data to improve the user experience. "
            "Do you want to help to improve Albert by sending telemetry data?"
        );

        auto informative_text = tr(
            "No tracking, profiling, sharing or commercial use. "
            "The transmitted data is non-personal. "
            "You can change this configuration anytime in the settings. "
            "See the <a href='https://albertlauncher.github.io/privacy/'>privacy notice</a> for details."
        );

        using MB = QMessageBox;
        MB mb;
        mb.setIcon(MB::Question);
        mb.setWindowTitle(qApp->applicationDisplayName());
        mb.setText(text);
        mb.setInformativeText(informative_text);
        mb.setStandardButtons(MB::Yes|MB::No);
        mb.setDefaultButton(MB::Yes);
        const auto enable = MB::Yes == mb.exec();
        enabled_ = enable;
        App::settings()->setValue(K_TELEMETRY_ENABLED, enable);
    }

    connect(&timer, &QTimer::timeout, this, [this] { trySendReport(); });

    timer.setInterval(60000);   // every minute

    if (enabled_)
        timer.start();
}

// TODO: Qt>6.7 use QRestReply
static expected<QJsonObject, QString> parseJsonObject(const QByteArray &bytes)
{
    QJsonParseError parse_error;
    const auto doc = QJsonDocument::fromJson(bytes, &parse_error);

    if (parse_error.error != QJsonParseError::NoError)
        return unexpected(parse_error.errorString());

    else if (!doc.isObject())
        return unexpected(u"Json document is not a JSON object."_s);

    else
        return doc.object();
}

static QCoro::Task<optional<QString>> sendReport(QJsonDocument report)
{
    // Not that optimal but there is no official domain and the IP has to be dynamic.

    // Get config
    auto request = QNetworkRequest(u"https://albertlauncher.github.io/config.json"_s);
    auto reply = unique_ptr<QNetworkReply>(network().get(request));
    co_await reply.get();
    if (reply->error() != QNetworkReply::NoError)
        co_return reply->errorString();

    // Parse config
    auto exp_obj = parseJsonObject(reply->readAll());
    if (!exp_obj)
        co_return exp_obj.error();

    // Extract endpoint
    const auto base_url = exp_obj->value("api_base_url").toString();
    if (base_url.isNull())
        co_return u"Config does not contain the api base url."_s;

    // Put report
    request = QNetworkRequest(base_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/json"_s);
    request.setHeader(QNetworkRequest::UserAgentHeader, u"Albert/"_s + qApp->applicationVersion());
    reply = unique_ptr<QNetworkReply>(
        network().put(request, report.toJson(QJsonDocument::Compact)));
    co_await reply.get();

    // On success return
    if (reply->error() == QNetworkReply::NoError)
        co_return nullopt;

    // On failure try to extract error message from response
    if (exp_obj = parseJsonObject(reply->readAll()); exp_obj)
        if (const auto msg = exp_obj->value("message").toString(); !msg.isEmpty())
            co_return msg;

    // Fallback to http status message
    co_return reply->errorString();
}

void Telemetry::trySendReport()
{
    // Skip if sent already today.
    // At 3 AM most people are asleep. Use it as the beginning of a "human day".
    auto now = QDateTime::currentDateTime();
    if (now.addSecs(-10800).date() > last_report.addSecs(-10800).date())
        QCoro::connect(sendReport(buildReport()), this, [now, this](auto &&opt_err) {
            if (opt_err)
                WARN << "Failed to send telemetry data:" << *opt_err;
            else
            {
                INFO << "Successfully sent telemetry data.";
                last_report = now;
                App::state()->setValue(K_LAST_REPORT, last_report);
            }
        });
}

QJsonObject Telemetry::albertTelemetry() const
{
    QJsonArray enabled_plugins;
    for (const auto &[id, plugin] : plugin_registry_.plugins())
        if (plugin.enabled)
            enabled_plugins.append(id);

    QJsonObject activationsSinceLastReport;
    for (const auto &[extension_id, activations] : UsageDatabase::instance().extensionActivationsSince(last_report))
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
    data.insert("albert", albertTelemetry());
    if (auto *apps_plugin = App::instance().extension<detail::TelemetryProvider>("applications");
        apps_plugin)
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
        App::settings()->setValue(K_TELEMETRY_ENABLED, enabled_);

        enabled_ ? timer.start() : timer.stop();
    }
}
