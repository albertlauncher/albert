// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <QDateTime>
#include <QObject>
#include <QString>
#include <albert/export.h>
#include <memory>
class QUrl;

namespace albert::util
{

///
/// Provides OAuth2 authentication with support for the Authorization Code Flow with PKCE and
/// refresh tokens.
///
class ALBERT_EXPORT OAuth2 : public QObject
{
    Q_OBJECT
public:

    OAuth2();
    ~OAuth2();

    void requestAccess();
    void updateTokens();

    const QString &clientId() const;
    void setClientId(const QString &);

    const QString &clientSecret() const;
    void setClientSecret(const QString &);

    const QString &scope() const;
    void setScope(const QString &);

    const QString &authUrl() const;
    void setAuthUrl(const QString &);

    const QString &redirectUri() const;
    void setRedirectUri(const QString &);

    bool isPkceEnabled() const;
    void setPkceEnabled(bool);

    const QString &tokenUrl() const;
    void setTokenUrl(const QString &);

    const QString &accessToken() const;
    const QString &refreshToken() const;
    const QDateTime &tokenExpiration() const;

    void setTokens(const QString &access_token,
                   const QString &refresh_token = {},
                   const QDateTime &expiration = {});

    const QString &error() const;

    enum class State {
        NotAuthorized,
        Awaiting,
        Granted
    };

    State state() const;

    void handleCallback(const QUrl &callback);

signals:

    void clientIdChanged(const QString &);
    void clientSecretChanged(const QString &);
    void scopeChanged(const QString &);
    void authUrlChanged(const QString &);
    void redirectUriChanged(const QString &);
    void tokenUrlChanged(const QString &);
    void tokensChanged();
    void stateChanged(State);

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
