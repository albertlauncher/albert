// Copyright (c) 2025-2025 Manuel Schneider

#include "networkutil.h"
#include <QDateTime>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

QNetworkAccessManager &albert::network()
{
    static thread_local QNetworkAccessManager network_manager;
    return network_manager;
}

QNetworkReply *albert::await(QNetworkReply *reply)
{
    if (reply->isFinished())
        return reply;
    else
    {
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        return reply;
    }
}

QString albert::percentEncoded(const QString &string)
{ return QString::fromUtf8(QUrl::toPercentEncoding(string)); }

QString albert::percentDecoded(const QString &string)
{ return QUrl::fromPercentEncoding(string.toUtf8()); }
