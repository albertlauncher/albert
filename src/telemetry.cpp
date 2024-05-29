// Copyright (C) 2014-2024 Manuel Schneider

#include "albert/logging.h"
#include "albert/util.h"
#include "telemetry.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>
static const char *CFG_LAST_REPORT = "last_report";
using namespace albert;


Telemetry::Telemetry()
{
    QObject::connect(&timer, &QTimer::timeout, &timer, [this]{trySendReport();});
    timer.setInterval(60000);
    timer.start();
}

void Telemetry::trySendReport()
{
    // timezones and daytimes of users make it complicated to get trustworthy per day data.
    // Therefore three hours sampling rate.

    auto ts = state()->value(CFG_LAST_REPORT, 0).toUInt();
    if (ts < QDateTime::currentSecsSinceEpoch() - 10800) {
        QJsonObject object = buildReport();
        QString addr = "Zffb,!!*\" $## $\"' **!";
        for (auto &c: addr)
            c.unicode()=c.unicode()+14;

        QNetworkRequest request((QUrl(addr)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
        QNetworkReply* reply = network()->put(request, QJsonDocument(object).toJson(QJsonDocument::Compact));
        QObject::connect(reply, &QNetworkReply::finished, [reply](){
            if (reply->error() == QNetworkReply::NoError){
                DEBG << "Report sent.";
                state()->setValue(CFG_LAST_REPORT, QDateTime::currentSecsSinceEpoch());
            }
            reply->deleteLater();
        });
    }
}

QJsonObject Telemetry::buildReport()
{
    QJsonObject object;
    object.insert("report", 1);
    object.insert("version", qApp->applicationVersion());
    object.insert("os", QSysInfo::prettyProductName());
    object.insert("id", QString::fromUtf8(QCryptographicHash::hash(QSysInfo::machineUniqueId(), QCryptographicHash::Sha1).toHex()).left(12));
    return object;
}

QString Telemetry::buildReportString()
{
    return QJsonDocument(buildReport()).toJson(QJsonDocument::Indented);
}
