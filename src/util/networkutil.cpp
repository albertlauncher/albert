// Copyright (c) 2025-2025 Manuel Schneider

#include "networkutil.h"
#include <QDateTime>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <mutex>
using namespace albert;
using namespace std;


QNetworkAccessManager &util::network()
{
    static thread_local QNetworkAccessManager network_manager;
    return network_manager;
}

QNetworkReply *util::await(QNetworkReply *reply)
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


class detail::RateLimiter::Private
{
public:
    const uint interval;
    mutex m;
    long long block_until = 0;

};

detail::RateLimiter::RateLimiter(unsigned int interval) :
    d(make_unique<Private>(interval))
{
}

detail::RateLimiter::~RateLimiter() = default;

const bool &detail::RateLimiter::debounce(const bool &valid)
{
    auto now = QDateTime::currentMSecsSinceEpoch();

    unique_lock lock(d->m);

    while (d->block_until > QDateTime::currentMSecsSinceEpoch())
        if (valid)
            QThread::msleep(10);
        else
            return valid;

    d->block_until = now + 1000;

    return valid;
}
