// Copyright (c) 2025-2025 Manuel Schneider

#include "filedownloader.h"
#include "logging.h"
#include "networkutil.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QUrl>
using namespace albert::util;
using namespace std;

FileDownloader::FileDownloader(const QUrl &_url, const QString &_path, QObject *_parent) :
    QObject(_parent),
    url(_url),
    path(_path)
{
}

void FileDownloader::start()
{
    QMetaObject::invokeMethod(this, [this]{ _start(); }, Qt::QueuedConnection);
}

void FileDownloader::_start()
{
    if (reply)
        return;

    DEBG << "Fetching file from" << url.toString();

    reply = network().get(QNetworkRequest(url));
    reply->setParent(this);

    connect(reply, &QNetworkReply::finished, this, [this] {
        QString error;

        if (reply->error() == QNetworkReply::NoError)
        {
            if (QSaveFile file(path); file.open(QIODevice::WriteOnly))
            {
                file.write(reply->readAll());
                if (auto success = file.commit(); !success)
                {
                    if (error = file.errorString(); error.isEmpty())
                        error = "Failed to write file. Error unknown.";
                }
            }
            else
            {
                if (error = file.errorString(); error.isEmpty())
                    error = "Failed to open file. Error unknown.";
            }
        }
        else
        {
            if (error = reply->errorString(); error.isEmpty())
                error = "Failed to download file. Unkown network error.";
        }

        if (error.isNull())
        {
            DEBG << "Download successful:" << url.toString() << path;
            emit finished(true, path);
        }
        else
        {
            DEBG << error << url.toString() << path;
            emit finished(false, error);
        }

        reply->deleteLater();
    });
}
