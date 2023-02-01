// Copyright (C) 2014-2023 Manuel Schneider

#include "albert/logging.h"
#include "telemetry.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>
static const char *CFG_LAST_REPORT = "last_report";
static const uint DEF_LAST_REPORT = 0;
static const char *CFG_TELEMETRY = "telemetry";


Telemetry::Telemetry()
{
    QObject::connect(&timer_, &QTimer::timeout, [this]{trySendReport();});

    QSettings settings(qApp->applicationName());
    if (!settings.contains(CFG_TELEMETRY)) {
        QMessageBox mb(QMessageBox::Question, "Albert telemetry",
                       "Albert collects anonymous data to improve user experience. You can check "
                       "the data sent in the details. Opt in?",
                       QMessageBox::No|QMessageBox::Yes);
        mb.setDefaultButton(QMessageBox::Yes);
        mb.setDetailedText(QJsonDocument(buildReport()).toJson(QJsonDocument::Indented));
        settings.setValue(CFG_TELEMETRY, mb.exec() == QMessageBox::Yes);
    }
    enable(settings.value(CFG_TELEMETRY).toBool());
}

void Telemetry::enable(bool enable)
{
    if (enable) {
        trySendReport();
        timer_.start(60000);
    }
    else {
        timer_.stop();
    }
    QSettings(qApp->applicationName()).setValue(CFG_TELEMETRY, enable);
}

bool Telemetry::isEnabled() const
{
    return QSettings(qApp->applicationName()).value(CFG_TELEMETRY).toBool();
}

void Telemetry::trySendReport()
{
    // timezones and daytimes of users make it complicated to get trustworthy per day data.
    // Therefore three hours sampling rate.

    auto ts = QSettings(qApp->applicationName()).value(CFG_LAST_REPORT, DEF_LAST_REPORT).toUInt();
    if (ts < QDateTime::currentSecsSinceEpoch() - 10800) {
        QJsonObject object = buildReport();
        QString addr = "Zffb,!!*\" $## $\"' **!";
        for ( auto &c: addr)
            c.unicode()=c.unicode()+14;

        static QNetworkAccessManager *manager = new QNetworkAccessManager;
        QNetworkRequest request((QUrl(addr)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
        QNetworkReply* reply = manager->put(request, QJsonDocument(object).toJson(QJsonDocument::Compact));
        QObject::connect(reply, &QNetworkReply::finished, [reply](){
            if (reply->error() == QNetworkReply::NoError){
                DEBG << "Report sent.";
                QSettings(qApp->applicationName()).setValue(CFG_LAST_REPORT, QDateTime::currentSecsSinceEpoch());
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
