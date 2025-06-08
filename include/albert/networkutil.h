// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <albert/export.h>
#include <memory>
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

}

namespace albert::detail {

/// Blocks execution
class ALBERT_EXPORT RateLimiter
{
public:
    RateLimiter(unsigned int ms);
    ~RateLimiter();

    /// Blocks until the next request is allowed.
    /// If `valid` is false, it returns immediately.
    const bool &debounce(const bool &valid);

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
