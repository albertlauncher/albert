// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/extensionregistry.h"
#include "appqueryhandler.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QString>
#include <QUrl>
#include <QCoreApplication>
using namespace albert;
using namespace std;

const QStringList AppQueryHandler::icon_urls{QStringLiteral(":app_icon")};

AppQueryHandler::AppQueryHandler(albert::ExtensionRegistry *registry) : registry_(registry)
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
                    [](){ showSettings(); }
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
                    [](){ quit(); }
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
                    [](){ restart(); }
                }
            }),

        StandardItem::make(
            "cache",
            tr("Cache location"),
            tr("Albert cache location"),
            icon_urls,
            {
                {
                    "cache",
                    tr("Open"),
                    [](){ albert::openUrl(QUrl::fromLocalFile(albert::cacheLocation())); }
                }
            }),

        StandardItem::make(
            "config",
            tr("Config location"),
            tr("Albert config location"),
            icon_urls,
            {
                {
                    "config",
                    tr("Open"),
                    [](){ albert::openUrl(QUrl::fromLocalFile(albert::configLocation())); }
                }
            }),

        StandardItem::make(
            "data",
            tr("Data location"),
            tr("Albert data location"),
            icon_urls,
            {
                {
                    "data",
                    tr("Open"),
                    [](){ albert::openUrl(QUrl::fromLocalFile(albert::dataLocation())); }
                }
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

vector<RankItem> AppQueryHandler::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> rank_items;

    for (const auto &item : items_)
        if (item->text().startsWith(query->string(), Qt::CaseInsensitive))
            rank_items.emplace_back(item, (float)query->string().length() / item->text().length());

    static const auto tr_enabled = tr("Enabled");
    static const auto tr_disabled = tr("Disabled");
    static const auto tr_toggle = tr("Toggle");
    static const auto tr_prop = tr("Property of the extension '%1' [%2]");

    for (auto &[id, qobject] : registry_->extensions<QObject>()){
        auto *metaObj = qobject->metaObject();
        for (int i = metaObj->propertyOffset(); i < metaObj->propertyCount(); ++i) {
            auto *extension = dynamic_cast<Extension*>(qobject);
            auto metaProp = metaObj->property(i);
            QString metaPropName{metaProp.name()};
            if (metaPropName.contains(query->string(), Qt::CaseInsensitive)){
                if(metaProp.isUser()){
                    if (metaProp.typeId() == QMetaType::fromType<bool>().id()) {
                        rank_items.emplace_back(
                            StandardItem::make(
                                QString("%1_%2").arg(id, metaPropName),
                                QString("%1: %2").arg(metaPropName,
                                                      metaProp.read(qobject).value<bool>()
                                                          ? tr_enabled : tr_disabled),
                                tr_prop.arg(extension->name(), metaProp.typeName()),
                                icon_urls,
                                {
                                    {
                                        "toggle",
                                        tr_toggle,
                                        [metaProp, q=qobject](){ metaProp.write(q, !metaProp.read(q).toBool()); }
                                    }
                                }
                            ),
                            (float)query->string().length() / metaPropName.length()
                        );
                    }
                }
            }
        }
    }

    return rank_items;
}
