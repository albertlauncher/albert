// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <memory>
class QNetworkReply;
class QString;
class QUrl;

namespace albert
{

///
/// Downloads a file from the given URL to the given path.
///
/// Does not check if the file exists. The download will fail to save the file in this case.
///
/// \ingroup util_net
///
class ALBERT_EXPORT Download : public QObject
{
    Q_OBJECT
public:

    ///
    /// Constructs a download with the given _url_, _path_ and _parent_.
    ///
    Download(const QUrl &url, const QString &path, QObject *parent = nullptr);

    ///
    /// Destructs this download.
    ///
    ~Download();

    ///
    /// Returns a unique download for _url_ and _path_.
    ///
    /// If a download for the same URL and path already exists, it is returned;
    /// otherwise a new download is created and returned.
    /// The download created is automatically started and lives in the main thread.
    ///
    /// This function is thread-safe.
    ///
    static std::shared_ptr<Download> unique(const QUrl &url, const QString &path);

    ///
    /// Returns the url of the download.
    ///
    /// This function is thread-safe.
    ///
    const QUrl &url();

    ///
    /// Returns the destination path of the download.
    ///
    /// This function is thread-safe.
    ///
    const QString &path();

    ///
    /// Returns true if the download is active.
    ///
    /// This function is thread-safe.
    ///
    bool isActive();

    ///
    /// Returns the error of the download, if any.
    ///
    /// This function is thread-safe.
    ///
    const QString &error();

    ///
    /// Starts the download.
    ///
    /// Do **not** move the download to another thread **after** calling this method.
    ///
    void start();

signals:

    ///
    /// Emitted when the download has finished.
    ///
    void finished();

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
