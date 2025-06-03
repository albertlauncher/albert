// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <memory>
class QNetworkReply;
class QString;
class QUrl;

namespace albert::util
{

/// Downloads a file from the given URL to the given path.
/// Does not check if the file exists. The download will fail to save the file in this case.
class ALBERT_EXPORT Download : public QObject
{
    Q_OBJECT
public:

    /// Constructs a download with the given _url_ and _path_. Threadsafe.
    Download(const QUrl &url, const QString &path, QObject *parent = nullptr);
    ~Download();

    /// Returns a unique download for _url_ and _path_.
    /// If a download for the same URL and path already exists, it is returned.
    /// Otherwise a new download is created and returned.
    /// The download returned is started and lives in the main thread and
    /// Threadsafe.
    static std::shared_ptr<Download> unique(const QUrl &url, const QString &path);

    /// Returns the url of the download. Threadsafe.
    const QUrl &url();

    /// Returns the destination path of the download. Threadsafe.
    const QString &path();

    /// Returns true if the download is active. Threadsafe.
    bool isActive();

    /// Returns the error of the download, if any. Threadsafe.
    const QString &error();

    /// Starts the download.
    /// The implementation is actually invoked in the thread this download lives in.
    /// Do _not_ move the download to another thread _after_ calling this method.
    void start();

signals:

    void finished();

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
