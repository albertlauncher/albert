// Copyright (c) 2025-2025 Manuel Schneider

#include "networkutil.h"
#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
using namespace albert;
using namespace std;


QNetworkAccessManager &util::network()
{
    static thread_local QNetworkAccessManager network_manager;
    return network_manager;
}

QNetworkReply *util::await(QNetworkReply *reply)
{
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    return reply;
}

QNetworkRequest util::makeRestRequest(const QString &base_url,
                                      const QString &path,
                                      const QUrlQuery &query,
                                      const QByteArray &authorization_header)
{
    QUrl url(base_url);
    url.setPath(path);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    if (!authorization_header.isNull())
        request.setRawHeader("Authorization", authorization_header);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return request;
}
