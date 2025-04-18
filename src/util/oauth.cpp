// Copyright (c) 2025-2025 Manuel Schneider

#include "albert.h"
#include "logging.h"
#include "networkutil.h"
#include "oauth.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
using enum albert::util::OAuth2::State;
using namespace albert::util;
using namespace albert;
using namespace std;

static QString generateRandomString(int length) {
    static const QString chars = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                "abcdefghijklmnopqrstuvwxyz"
                                                "0123456789");
    QString result;
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        int idx = QRandomGenerator::global()->bounded(chars.size());
        result.append(chars[idx]);
    }
    return result;
}

static QString generateCodeChallenge(const QString &code_verifier) {
    QByteArray hash = QCryptographicHash::hash(code_verifier.toUtf8(), QCryptographicHash::Sha256);
    QByteArray b64 = hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return QString::fromUtf8(b64);
}

class OAuth2::Private
{
public:
    OAuth2 *q;

    QString client_id;
    QString client_secret;
    QString scope;
    QString token_url;

    // Stage 1 Grant permissions

    QString auth_url;
    QString redirect_uri;
    QString code_verifier;
    QString state_string;
    bool pkce = true;

    // Stage 2 Authorize access

    QString refresh_token;
    QString access_token;
    QDateTime token_expiration;
    QTimer token_refresh_timer;

    QString error;

    void addBasicAuthorizationHeader(QNetworkRequest *) const;
    void authorizeWithCode(const QString &code);
    void refreshAccessToken();
    void parseTokenReply(QNetworkReply *reply);
};


OAuth2::OAuth2() : d(make_unique<Private>(this))
{
    connect(&d->token_refresh_timer, &QTimer::timeout, this, &OAuth2::updateTokens);
    d->token_refresh_timer.setSingleShot(true);
}

OAuth2::~OAuth2() {}

void OAuth2::requestAccess()
{
    d->state_string = generateRandomString(8);

    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("client_id", d->client_id);
    query.addQueryItem("scope", d->scope);
    query.addQueryItem("redirect_uri", d->redirect_uri);
    query.addQueryItem("state", d->state_string);
    if (isPkceEnabled())
    {
        d->code_verifier = generateRandomString(64);
        auto code_challenge = generateCodeChallenge(d->code_verifier);
        query.addQueryItem("code_challenge_method", "S256");
        query.addQueryItem("code_challenge", code_challenge);
    }

    QUrl url(d->auth_url);
    url.setQuery(query);
    open(url);

    emit stateChanged(Awaiting);
}

void OAuth2::handleCallback(const QUrl &callback_url)
{
    QUrlQuery url_query(callback_url.query());

    if (d->state_string.isEmpty())
        return;  // Unexpected OAuth callback

    else if (const auto state_string = url_query.queryItemValue("state");
             state_string != d->state_string)
        return;  // state mismatch

    d->state_string.clear();

    if (url_query.hasQueryItem("error"))
    {
        d->error = url_query.queryItemValue("error");
        emit stateChanged(NotAuthorized);
    }

    else if (!url_query.hasQueryItem("code"))
    {
        d->error = "URL query does not contain 'code'.";
        emit stateChanged(NotAuthorized);
    }

    else if (const auto code = url_query.queryItemValue("code");
             code.isEmpty())
    {
        d->error = "'code' is empty.";
        emit stateChanged(NotAuthorized);
    }

    else
        d->authorizeWithCode(code);
}

void OAuth2::Private::addBasicAuthorizationHeader(QNetworkRequest *req) const
{
    const auto base64 = QString("%1:%2").arg(client_id, client_secret).toUtf8().toBase64();
    req->setRawHeader(QByteArray("Authorization"),
                      QString("Basic %1").arg(base64).toUtf8());
}

void OAuth2::Private::authorizeWithCode(const QString & code)
{
    QUrlQuery params;
    params.addQueryItem("grant_type", "authorization_code");
    params.addQueryItem("code", code);
    params.addQueryItem("redirect_uri", redirect_uri);
    QNetworkRequest request(token_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "application/json");
    if (pkce)
    {
        params.addQueryItem("client_id", client_id);
        params.addQueryItem("code_verifier", code_verifier);
    }
    else
        addBasicAuthorizationHeader(&request);

    QNetworkReply *reply = network().post(request, params.toString(QUrl::FullyEncoded).toUtf8());

    QObject::connect(reply, &QNetworkReply::finished, q, [this, reply]
    {
        parseTokenReply(reply);
        reply->deleteLater();
        if (!access_token.isEmpty())
            emit q->stateChanged(Granted);
    });
}


void OAuth2::Private::refreshAccessToken()
{
    QUrlQuery params;
    params.addQueryItem("grant_type", "refresh_token");
    params.addQueryItem("refresh_token", refresh_token);
    QNetworkRequest request(token_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    if (pkce)
        params.addQueryItem("client_id", client_id);
    else
        addBasicAuthorizationHeader(&request);

    QNetworkReply *reply = network().post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    QObject::connect(reply, &QNetworkReply::finished, q, [this, reply]
    {
        parseTokenReply(reply);
        reply->deleteLater();
    });
}


void OAuth2::Private::parseTokenReply(QNetworkReply *reply)
{
    // {
    //     access_token: 'BQBLuPRYBQ...BP8stIv5xr-Iwaf4l8eg',
    //     token_type: 'Bearer',
    //     expires_in: 3600,
    //     refresh_token: 'AQAQfyEFmJJuCvAFh...cG_m-2KTgNDaDMQqjrOa3',
    //     scope: 'user-read-email user-read-private'
    // }

    error.clear();

    if ((int)reply->error() == 302)  // Grant invalid/revoked
    {
        refresh_token.clear();

        error = QString("%1 (%2) - %3 %4")
                    .arg((int)reply->error())
                    .arg(reply->error())
                    .arg(reply->errorString(), reply->readAll());
    }

    else if (reply->error() != QNetworkReply::NoError)
    {
        error = QString("%1 (%2) - %3 %4")
        .arg((int)reply->error())
            .arg(reply->error())
            .arg(reply->errorString(), reply->readAll());
    }

    else if (auto obj = QJsonDocument::fromJson(reply->readAll()).object();
             obj.isEmpty())
        error = QString("Failed parsing response.");

    else if (const auto type = obj["token_type"].toString();
             QString::compare(type, QStringLiteral("bearer"), Qt::CaseInsensitive))
        error = QString("Unsupported token type: %1.").arg(type);

    // else if (const auto s = obj["scope"].toString();
    //          scope != s)
    //     error = QString("Scope mismatch: Expect: '%1' but got '%2'.")
    //                 .arg(scope, s);

    else if (const auto at  = obj["access_token"].toString();
             at.isEmpty())
        error = "Access token is empty.";

    else
    {
        q->setTokens(at,
                     obj["refresh_token"].toString(),
                     QDateTime::currentDateTime().addSecs(obj["expires_in"].toInt()));
        return;
    }

    error = QString("Access token update failed: %1").arg(error);
    access_token.clear();
    refresh_token.clear();
    token_expiration = {};
    emit q->tokensChanged();
}

void OAuth2::updateTokens() { d->refreshAccessToken(); }

// -------------------------------------------------------------------------------------------------

const QString &OAuth2::clientId() const { return d->client_id; }

void OAuth2::setClientId(const QString &v)
{
    if (v != d->client_id)
    {
        d->client_id = v;
        emit clientIdChanged(v);
    }
}

const QString &OAuth2::clientSecret() const { return d->client_secret; }

void OAuth2::setClientSecret(const QString &v)
{
    if (v != d->client_secret)
    {
        d->client_secret = v;
        emit clientSecretChanged(v);
    }
}

const QString &OAuth2::scope() const { return d->scope; }

void OAuth2::setScope(const QString &v)
{
    if (v != d->scope)
    {
        d->scope = v;
        emit scopeChanged(v);
    }
}

const QString &OAuth2::authUrl() const { return d->auth_url; }

void OAuth2::setAuthUrl(const QString &v)
{
    if (v != d->auth_url)
    {
        d->auth_url = v;
        emit authUrlChanged(v);
    }
}

const QString &OAuth2::redirectUri() const { return d->redirect_uri; }

void OAuth2::setRedirectUri(const QString &v)
{
    if (v != d->redirect_uri)
    {
        d->redirect_uri = v;
        emit redirectUriChanged(v);
    }
}

bool OAuth2::isPkceEnabled() const { return d->pkce; }

void OAuth2::setPkceEnabled(bool v)
{
    if (v != d->pkce)
    {
        d->pkce = v;
    }
}

const QString &OAuth2::tokenUrl() const { return d->token_url; }

void OAuth2::setTokenUrl(const QString &v)
{
    if (v != d->token_url)
    {
        d->token_url = v;
        emit tokenUrlChanged(v);
    }
}

const QString &OAuth2::accessToken() const { return d->access_token; }

const QString &OAuth2::refreshToken() const { return d->refresh_token; }

const QDateTime &OAuth2::tokenExpiration() const { return d->token_expiration; }

void OAuth2::setTokens(const QString &access_token,
                       const QString &refresh_token,
                       const QDateTime &expiration)
{
    const auto state_before = state();

    d->access_token = access_token;
    d->refresh_token = refresh_token;
    d->token_refresh_timer.stop();
    d->token_expiration = expiration;
    if (!refresh_token.isEmpty())
    {
        if (expiration.isNull())
            WARN << "Got 'refresh_token' but no valid expiration.";
        else
        {
            if (const auto expires_in = QDateTime::currentDateTime().secsTo(expiration);
                expires_in > 0)
                d->token_refresh_timer.start((expires_in - 30) * 1000);
            else
                d->refreshAccessToken();
        }
    }

    emit tokensChanged();

    const auto state_after = state();
    if (state_before != state_after)
        emit stateChanged(state_after);
}

const QString &OAuth2::error() const { return d->error; }

OAuth2::State OAuth2::state() const
{
    using enum State;
    if (!d->access_token.isEmpty()
        && (!d->token_expiration.isValid()
            || d->token_expiration > QDateTime::currentDateTime()))
        return Granted;
    else if (d->state_string.isEmpty())
        return NotAuthorized;
    else
        return Awaiting;
}
