// Copyright (c) 2023-2025 Manuel Schneider

#include "albert.h"
#include "appqueryhandler.h"
#include "matcher.h"
#include "standarditem.h"
#include "systemutil.h"
#include <QString>
#include <QUrl>
#include <albert/extensionregistry.h>
#include <albert/plugin/applications.h>
#include <filesystem>
using namespace albert::util;
using namespace albert;
using namespace std;

const QStringList AppQueryHandler::icon_urls{QStringLiteral(":app_icon")};

AppQueryHandler::AppQueryHandler():
    strings({
        .cache=tr("Cache location"),
        .cached=tr("Albert cache location"),
        .config=tr("Config location"),
        .configd=tr("Albert config location"),
        .data=tr("Data location"),
        .datad=tr("Albert data location"),
        .open=tr("Open"),
        .topen=tr("Open in terminal"),
        .quit=tr("Quit"),
        .quitd=tr("Quit Albert"),
        .restart=tr("Restart"),
        .restartd=tr("Restart Albert"),
        .settings=tr("Settings"),
        .settingsd=tr("Albert settings"),
    })
{

}

QString AppQueryHandler::id() const { return QStringLiteral("albert"); }

QString AppQueryHandler::name() const { return QStringLiteral("Albert"); }

QString AppQueryHandler::description() const { return tr("Control the app"); }

QString AppQueryHandler::defaultTrigger() const { return QStringLiteral("albert "); }

static void openTermAt(applications::Plugin *apps, const std::filesystem::path &loc)
{
    apps->runTerminal(QStringLiteral("cd '%1'; exec $SHELL")
                          .arg(QString::fromLocal8Bit(loc.c_str())));
}

vector<RankItem> AppQueryHandler::handleGlobalQuery(const Query &query)
{
    Matcher matcher(query.string());
    vector<RankItem> rank_items;
    Match m;

    if (m = matcher.match(strings.settings); m)
        rank_items.emplace_back(
            StandardItem::make(
                QStringLiteral("sett"),
                strings.settings,
                strings.settingsd,
                icon_urls,
                {{ QStringLiteral("open"), strings.open, [] { showSettings(); } }}),
            m
        );

    if (m = matcher.match(strings.quit); m)
        rank_items.emplace_back(
        StandardItem::make(
            QStringLiteral("quit"),
            strings.quit,
            strings.quitd,
            icon_urls,
            {{ QStringLiteral("quit"), strings.quit, [] { quit(); } }}),
            m
        );

    if (m = matcher.match(strings.restart); m)
        rank_items.emplace_back(
            StandardItem::make(
                QStringLiteral("restart"),
                strings.restart,
                strings.restartd,
                icon_urls,
                {{ QStringLiteral("restart"), strings.restart, [] { restart(); } }}),
            m
        );

    applications::Plugin *apps = nullptr;
    try {
        // TODO use WeakDep
        apps = dynamic_cast<applications::Plugin*>(extensionRegistry().extensions()
                                                        .at(QStringLiteral("applications")));
    } catch (const std::out_of_range &) { /* okay, optional */ }

    if (m = matcher.match(strings.cache); m)
    {
        vector<Action> actions = {{QStringLiteral("open"), strings.open,
                                   []{ open(cacheLocation()); }}};
        if (apps)
            actions.emplace_back(QStringLiteral("term"), strings.topen,
                                 [=]{ openTermAt(apps, cacheLocation()); });

        auto i = rank_items.emplace_back(
            StandardItem::make(QStringLiteral("cache"), strings.cache,
                               strings.cached, icon_urls, actions), m);
    }

    if (m = matcher.match(strings.config); m)
    {
        vector<Action> actions = {{ QStringLiteral("open"), strings.open,
                                   [] { open(configLocation()); } }};
        if (apps)
            actions.emplace_back(QStringLiteral("term"), strings.topen,
                                 [=]{ openTermAt(apps, configLocation()); });

        auto i = rank_items.emplace_back(
            StandardItem::make(QStringLiteral("config"), strings.config, strings.configd,
                               icon_urls, actions), m);
    }

    if (m = matcher.match(strings.data); m)
    {
        vector<Action> actions = {{ QStringLiteral("open"), strings.open,
                                   [] { open(dataLocation()); } }};
        if (apps)
            actions.emplace_back(QStringLiteral("term"), strings.topen,
                                 [=]{ openTermAt(apps, dataLocation()); });

        auto i = rank_items.emplace_back(
            StandardItem::make(QStringLiteral("data"), strings.data, strings.datad,
                               icon_urls, actions), m);
    }

    return rank_items;
}
