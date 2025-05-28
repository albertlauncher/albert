// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <albert/export.h>
class QByteArray;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;
class QUrlQuery;

namespace albert::util
{

///
/// Returns a global, threadlocal QNetworkAccessManager.
///
ALBERT_EXPORT QNetworkAccessManager &network();

///
/// Blocks until `reply` is finished.
///
ALBERT_EXPORT QNetworkReply *await(QNetworkReply *reply);


ALBERT_EXPORT QNetworkRequest makeRestRequest(const QString &base_url,
                                              const QString &path,
                                              const QUrlQuery &query,
                                              const QByteArray &authorization_header);

}
