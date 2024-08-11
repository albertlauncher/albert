// Copyright (c) 2023-2024 Manuel Schneider

#include "app.h"
#include "appqueryhandler.h"
#include "matcher.h"
#include "standarditem.h"
#include "util.h"
#include <QString>
#include <QUrl>
using namespace albert;
using namespace std;

const QStringList AppQueryHandler::icon_urls{QStringLiteral(":app_icon")};

AppQueryHandler::AppQueryHandler()
{
    items_ = {
        StandardItem::make(
            "sett",
            tr("Settings"),
            tr("Albert settings"),
            icon_urls,
            {
                {
                    "sett",
                    tr("Open"),
                    [](){ App::instance()->showSettings(); }
                }
            }),

        StandardItem::make(
            "quit",
            tr("Quit"),
            tr("Quit Albert"),
            icon_urls,
            {
                {
                    "quit",
                    tr("Quit"),
                    [](){ App::instance()->quit(); }
                }
            }),

        StandardItem::make(
            "restart",
            tr("Restart"),
            tr("Restart Albert"),
            icon_urls,
            {
                {
                    "restart",
                    tr("Restart"),
                    [](){ App::instance()->restart(); }
                }
            }),

        StandardItem::make(
            "cache",
            tr("Cache location"),
            tr("Albert cache location"),
            icon_urls,
            {
                {
                    "cache", tr("Open"),
                    []{ albert::openUrl(QUrl::fromLocalFile(albert::cacheLocation())); }
                },
            }),

        StandardItem::make(
            "config",
            tr("Config location"),
            tr("Albert config location"),
            icon_urls,
            {
                {
                    "config", tr("Open"),
                    [](){ albert::openUrl(QUrl::fromLocalFile(albert::configLocation())); }
                },
            }),

        StandardItem::make(
            "data",
            tr("Data location"),
            tr("Albert data location"),
            icon_urls,
            {
                {
                    "data", tr("Open"),
                    [](){ albert::openUrl(QUrl::fromLocalFile(albert::dataLocation())); }
                },
            }),
    };
}

QString AppQueryHandler::id() const
{ return QStringLiteral("albert"); }

QString AppQueryHandler::name() const
{ return QStringLiteral("Albert"); }

QString AppQueryHandler::description() const
{ return tr("Control the app"); }

QString AppQueryHandler::defaultTrigger() const
{ return QStringLiteral("albert "); }

vector<RankItem> AppQueryHandler::handleGlobalQuery(const Query *query) const
{
    Matcher matcher(query->string());
    vector<RankItem> rank_items;
    for (const auto &item : items_)
        if (auto m = matcher.match(item->text()); m)
            rank_items.emplace_back(item, m);
    return rank_items;
}
