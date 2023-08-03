// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/extensionregistry.h"
#include "appqueryhandler.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QString>
#include <QUrl>
using namespace albert;
using namespace std;

AppQueryHandler::AppQueryHandler(albert::ExtensionRegistry *registry) : registry_(registry) {}

QString AppQueryHandler::id() const { return QStringLiteral("albert"); }

QString AppQueryHandler::name() const { return QStringLiteral("Albert"); }

QString AppQueryHandler::description() const { return QStringLiteral("Control the app"); }

QString AppQueryHandler::defaultTrigger() const { return QStringLiteral("app "); }

vector<RankItem> AppQueryHandler::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> items;

    auto str_settings = QStringLiteral("settings");
    auto str_quit = QStringLiteral("quit");
    auto str_restart = QStringLiteral("restart");
    auto str_config = QStringLiteral("config");
    auto str_data = QStringLiteral("data");
    auto str_cache = QStringLiteral("cache");

    if (str_settings.startsWith(query->string(), Qt::CaseInsensitive)){
        items.emplace_back(
            StandardItem::make(
                "albert-settings",
                "Albert settings",
                "Open the Albert settings window",
                {":app_icon"},
                {{"albert-settings", "Open settings", [](){ showSettings(); }}}
            ),
            (float)query->string().length() / str_settings.length()
        );
    }

    else if (str_quit.startsWith(query->string(), Qt::CaseInsensitive)){
        items.emplace_back(
            StandardItem::make(
                "albert-quit",
                "Quit Albert",
                "Quit this application",
                {":app_icon"},
                {{"albert-quit", "Quit Albert", [](){ quit(); }}}
            ),
            (float)query->string().length() / str_quit.length()
        );
    }

    else if (str_restart.startsWith(query->string(), Qt::CaseInsensitive)){
        items.emplace_back(
            StandardItem::make(
                "albert-restart",
                "Restart Albert",
                "Restart this application",
                {":app_icon"},
                {{"albert-restart", "Restart Albert", [](){ restart(); }}}
            ),
            (float)query->string().length() / str_restart.length()
        );
    }

    else if (str_config.startsWith(query->string(), Qt::CaseInsensitive)){
        items.emplace_back(
            StandardItem::make(
                "albert-config",
                "Albert config location",
                albert::configLocation(),
                {":app_icon"},
                {{"open", "Open", [](){ albert::openUrl(QUrl::fromLocalFile(albert::configLocation())); }}}
            ),
            (float)query->string().length() / str_config.length()
        );
    }

    else if (str_data.startsWith(query->string(), Qt::CaseInsensitive)){
        items.emplace_back(
            StandardItem::make(
                "albert-data",
                "Albert data location",
                albert::dataLocation(),
                {":app_icon"},
                {{"open", "Open", [](){ albert::openUrl(QUrl::fromLocalFile(albert::dataLocation())); }}}
            ),
            (float)query->string().length() / str_data.length()
        );
    }

    else if (str_cache.startsWith(query->string(), Qt::CaseInsensitive)){
        items.emplace_back(
            StandardItem::make(
                "albert-cache",
                "Albert cache location",
                albert::cacheLocation(),
                {":app_icon"},
                {{"open", "Open", [](){ albert::openUrl(QUrl::fromLocalFile(albert::cacheLocation())); }}}
            ),
            (float)query->string().length() / str_cache.length()
        );
    }

    for (auto &[id, qobject] : registry_->extensions<QObject>()){
        auto *metaObj = qobject->metaObject();
        for (int i = metaObj->propertyOffset(); i < metaObj->propertyCount(); ++i) {
            auto *extension = dynamic_cast<Extension*>(qobject);
            auto metaProp = metaObj->property(i);
            QString metaPropName{metaProp.name()};
            if (metaPropName.contains(query->string(), Qt::CaseInsensitive)){
                if(metaProp.isUser()){
                    if (metaProp.typeId() == QMetaType::fromType<bool>().id()) {
                        items.emplace_back(
                            StandardItem::make(
                                QString("%1_%2").arg(id, metaPropName),
                                QString("%1: %2").arg(metaPropName, metaProp.read(qobject).value<bool>() ? "Enabled" : "Disabled"),
                                QString("%1 property [%2]").arg(extension->name(), metaProp.typeName()),
                                {":app_icon"},
                                {{"toggle", "Toggle", [metaProp, qobject=qobject](){ metaProp.write(qobject, !metaProp.read(qobject).toBool()); }}}
                            ),
                            (float)query->string().length() / metaPropName.length()
                        );
                    }
                }
            }
        }
    }

    return items;
}
