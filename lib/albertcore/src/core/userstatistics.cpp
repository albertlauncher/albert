// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSysInfo>
#include <QTimer>
#include "usagedatabase.h"
#include "userstatistics.h"
using namespace Core;
using namespace std;


/** ***********************************************************************************************/
UserStatistics::UserStatistics() {
    QObject::connect(&timer_, &QTimer::timeout, this, &UserStatistics::checkSend);
    timer_.start(60000);
    checkSend();
}


/** ***********************************************************************************************/
void UserStatistics::checkSend() {
    int64_t secsSinceEpoch = QSettings(qApp->applicationName()).value("last_report", 0).toLongLong();
    if (QDateTime::fromMSecsSinceEpoch(secsSinceEpoch*1000).date() == QDate::currentDate()) return;
    sendSince(secsSinceEpoch);
}


/** ***********************************************************************************************/
void UserStatistics::sendSince(int64_t secsSinceEpoch) {

    QJsonObject object;
    object.insert("version", qApp->applicationVersion());
    object.insert("os", QSysInfo::prettyProductName());
    object.insert("os_version", QSysInfo::productVersion());
    object.insert("activations", static_cast<qint64>(UsageDatabase::activationsSince(secsSinceEpoch)));

    QString addr = "Zffb,!!*\" $## $\"' **!";
    for ( auto &c: addr)
        c.unicode()=c.unicode()+14;

    QNetworkAccessManager *manager = new QNetworkAccessManager;

    QNetworkRequest request((QUrl(addr)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    QNetworkReply* reply = manager->put(request, QJsonDocument(object).toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, [reply](){
        if (reply->error() == QNetworkReply::NoError){
            QSettings(qApp->applicationName()).setValue("last_report", QDateTime::currentMSecsSinceEpoch()/1000);
        }
        reply->deleteLater();
    });
}
