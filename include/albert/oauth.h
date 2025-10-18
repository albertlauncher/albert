// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <QDateTime>
#include <QObject>
#include <QString>
#include <albert/export.h>
#include <memory>
class QUrl;

namespace albert
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

    /// Requests access, i.e. starts the Authorization Code Flow to obtain an access token.
    void requestAccess();

    /// Updates the access token.
    void updateTokens();

    /// Returns the client identifier.
    const QString &clientId() const;

    /// Sets the client identifier to _id_.
    void setClientId(const QString &id);

    /// Returns the client secret.
    const QString &clientSecret() const;

    /// Sets the client secret to _secret_.
    void setClientSecret(const QString &secret);

    /// Returns the OAuth scope to request permissions for.
    const QString &scope() const;

    /// Sets the OAuth scope to request permissions for to _scope_.
    void setScope(const QString &scope);

    /// Returns the authorization URL.
    const QString &authUrl() const;

    /// Sets the authorization URL to _url_.
    void setAuthUrl(const QString &url);

    /// Returns the redirect URI.
    const QString &redirectUri() const;

    /// Sets the redirect URI to _uri_.
    void setRedirectUri(const QString &uri);

    /// Returns true if PKCE is enabled, false otherwise.
    bool isPkceEnabled() const;

    /// Sets whether PKCE is enabled or not.
    void setPkceEnabled(bool enabled);

    /// Returns the token URL.
    const QString &tokenUrl() const;

    /// Sets the token URL to _url_.
    void setTokenUrl(const QString &url);

    /// Returns the access token.
    const QString &accessToken() const;

    /// Returns the access token.
    const QString &refreshToken() const;

    /// Returns the access token.
    const QDateTime &tokenExpiration() const;

    /// Sets the access token, refresh token and expiration date.
    void setTokens(const QString &access_token,
                   const QString &refresh_token = {},
                   const QDateTime &expiration = {});

    /// Returns the error message if any.
    const QString &error() const;

    enum class State {
        NotAuthorized,  ///< Not yet authorized.
        Awaiting,  ///< Waiting for user interaction to authorize.
        Granted  ///< Authorization granted and access token available.
    };

    /// Returns the state of the authorization flow.
    State state() const;

    /// Handles the redirect callback URL from the OAuth2 provider.
    void handleCallback(const QUrl &callback);

signals:

    void clientIdChanged(const QString &);  ///< Emitted when the client ID changes.
    void clientSecretChanged(const QString &);  ///< Emitted when the client secret changes.
    void scopeChanged(const QString &);  ///< Emitted when the scope changes.
    void authUrlChanged(const QString &);  ///< Emitted when the authorization URL changes.
    void redirectUriChanged(const QString &);  ///< Emitted when the redirect URI changes.
    void tokenUrlChanged(const QString &);  ///< Emitted when the token URL changes.
    void tokensChanged();  ///< Emitted when the access token, refresh token or expiration date changes.
    void stateChanged(State);  ///< Emitted when the state changes.

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
