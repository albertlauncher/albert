// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <memory>
class QByteArray;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QString;
class QUrlQuery;

/// \defgroup util_net Network utility
/// \ingroup util
/// Utility classes and helper functions for network related tasks.

namespace albert
{

/// \addtogroup util_net
/// @{

///
/// Returns a global, threadlocal QNetworkAccessManager.
///
ALBERT_EXPORT QNetworkAccessManager &network();

///
/// Blocks until _reply_ is finished.
///
ALBERT_EXPORT QNetworkReply *await(QNetworkReply *reply);

///
/// Returns _string_ percent encoded.
///
ALBERT_EXPORT QString percentEncoded(const QString &string);

///
/// Returns _string_ percent decoded.
///
ALBERT_EXPORT QString percentDecoded(const QString &string);

/// @}

}

namespace albert::detail {

///
/// Blocks execution
///
class ALBERT_EXPORT RateLimiter
{
public:
    RateLimiter(unsigned int ms);
    ~RateLimiter();

    ///
    /// Blocks until the next request is allowed or `valid` is false.
    ///
    const bool &debounce(const bool &valid);

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
