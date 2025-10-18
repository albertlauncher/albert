// Copyright (c) 2025-2025 Manuel Schneider

#include "download.h"
#include "logging.h"
#include "networkutil.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QUrl>
#include <QDir>
#include <map>
#include <memory>
#include <mutex>
#include <QTimer>
using namespace albert;
using namespace std;

class Download::Private
{
public:
    const QUrl url;
    const QString path;
    QNetworkReply *reply;
    QString error;
    mutex instance_mutex;
};

Download::Download(const QUrl &_url, const QString &_path, QObject *_parent) :
    QObject(_parent),
    d(make_unique<Private>(_url, _path))
{
}

Download::~Download() = default;

const QUrl &Download::url() { return d->url; }  // threadsafe, const.

const QString &Download::path() { return d->path; }  // threadsafe, const.

bool Download::isActive()
{
    lock_guard<mutex> lock(d->instance_mutex);
    return d->reply;
}

const QString &Download::error()
{
    lock_guard<mutex> lock(d->instance_mutex);
    return d->error;
}

void Download::start()
{
    if (d->reply)
    {
        WARN << "Download already in progress:" << d->url.toString();
        return;
    }

    QMetaObject::invokeMethod(qApp, [this] // Invoke on the main thread
    {
        DEBG << "Start download from" << d->url.toString();

        lock_guard<mutex> lock(d->instance_mutex);
        d->reply = network().get(QNetworkRequest(d->url));
        d->reply->setParent(this);

        connect(d->reply, &QNetworkReply::finished, this, [this]
        {
            d->instance_mutex.lock();

            if (d->reply->error() == QNetworkReply::NoError)
            {
                QFileInfo info(d->path);

                if (info.exists())
                    d->error = "File already exists.";

                else if (auto dir = info.dir();
                         !dir.mkpath("."))
                    d->error = "Cannot create parent directory.";

                if (QSaveFile file(d->path); file.open(QIODevice::WriteOnly))
                {
                    file.write(d->reply->readAll());
                    if (auto success = file.commit(); !success)
                    {
                        if (d->error = file.errorString(); d->error.isEmpty())
                            d->error = "Failed to write file. Error unknown.";
                    }
                }
                else
                {
                    if (d->error = file.errorString(); d->error.isEmpty())
                        d->error = "Failed to open file. Error unknown.";
                }
            }
            else
            {
                if (d->error = d->reply->errorString(); d->error.isEmpty())
                    d->error = "Failed to download file. Unkown network error.";
            }

            if (d->error.isNull())
                DEBG << "Download successful:" << d->url.toString() << d->path;
            else
                DEBG << "Download failed:" << d->error << d->url.toString() << d->path;

            d->reply->deleteLater();
            d->reply = nullptr;

            d->instance_mutex.unlock();

            // QTimer::singleShot(1s, this, [this]{ emit finished(); });
            emit finished();

        });

    }, Qt::QueuedConnection);
}

shared_ptr<Download> Download::unique(const QUrl &url, const QString &path)
{
    static mutex static_mutex;
    static map<pair<QUrl, QString>, shared_ptr<Download>> active_downloads;

    const auto key = make_pair(url, path);
    lock_guard<mutex> lock(static_mutex);

    if (auto it = active_downloads.find(key); it == active_downloads.end())
    {
        auto sd = shared_ptr<Download>(new Download(url, path),
                                       [](Download *dl) { dl->deleteLater(); });

        sd->moveToThread(qApp->thread());

        active_downloads.emplace(key, sd);

        QObject::connect(sd.get(), &Download::finished, [key]
        {
            lock_guard<mutex> inner_lock(static_mutex);
            active_downloads.erase(key);
        });

        sd->start();

        return sd;
    }
    else
    {
        return it->second;
    }
}
