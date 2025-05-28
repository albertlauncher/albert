// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <QString>
#include <QUrl>
#include <albert/export.h>
class QNetworkReply;

namespace albert::util
{

class ALBERT_EXPORT FileDownloader : public QObject
{
    Q_OBJECT
public:

    FileDownloader(const QUrl &url, const QString &path, QObject *parent = nullptr);

    void start();

    const QUrl url;
    const QString path;

signals:

    void finished(bool success, const QString &path_or_error);

private:

    Q_INVOKABLE void _start();
    QNetworkReply *reply;

};

}
