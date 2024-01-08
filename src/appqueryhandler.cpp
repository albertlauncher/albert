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
            QCoreApplication::translate("AppQueryHandler", "Settings"),
            QCoreApplication::translate("AppQueryHandler", "Albert settings"),
            icon_urls,
            {
                {
                    "sett",
                    QCoreApplication::translate("AppQueryHandler", "Open"),
                    [](){ showSettings(); }
                }
            }),

        StandardItem::make(
            "quit",
            QCoreApplication::translate("AppQueryHandler", "Quit"),
            QCoreApplication::translate("AppQueryHandler", "Quit Albert"),
            icon_urls,
            {
                {
                    "quit",
                    QCoreApplication::translate("AppQueryHandler", "Quit"),
                    [](){ quit(); }
                }
            }),

        StandardItem::make(
            "restart",
            QCoreApplication::translate("AppQueryHandler", "Restart"),
            QCoreApplication::translate("AppQueryHandler", "Restart Albert"),
            icon_urls,
            {
                {
                    "restart",
                    QCoreApplication::translate("AppQueryHandler", "Restart"),
                    [](){ restart(); }
                }
            }),

        StandardItem::make(
            "cache",
            QCoreApplication::translate("AppQueryHandler", "Cache location"),
            QCoreApplication::translate("AppQueryHandler", "Albert cache location"),
            icon_urls,
            {
                {
                    "cache",
                    QCoreApplication::translate("AppQueryHandler", "Open"),
                    [](){ albert::openUrl(QUrl::fromLocalFile(albert::cacheLocation())); }
                }
            }),

        StandardItem::make(
            "config",
            QCoreApplication::translate("AppQueryHandler", "Config location"),
            QCoreApplication::translate("AppQueryHandler", "Albert config location"),
            icon_urls,
            {
                {
                    "config",
                    QCoreApplication::translate("AppQueryHandler", "Open"),
                    [](){ albert::openUrl(QUrl::fromLocalFile(albert::configLocation())); }
                }
            }),

        StandardItem::make(
            "data",
            QCoreApplication::translate("AppQueryHandler", "Data location"),
            QCoreApplication::translate("AppQueryHandler", "Albert data location"),
            icon_urls,
            {
                {
                    "data",
                    QCoreApplication::translate("AppQueryHandler", "Open"),
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
{ return QCoreApplication::translate("AppQueryHandler", "Control the app"); }

QString AppQueryHandler::defaultTrigger() const
{ return QStringLiteral("albert "); }

vector<RankItem> AppQueryHandler::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> rank_items;

    for (const auto &item : items_)
        if (item->text().startsWith(query->string(), Qt::CaseInsensitive))
            rank_items.emplace_back(item, (float)query->string().length() / item->text().length());

    static const auto tr_enabled = QCoreApplication::translate("AppQueryHandler", "Enabled");
    static const auto tr_disabled = QCoreApplication::translate("AppQueryHandler", "Disabled");
    static const auto tr_toggle = QCoreApplication::translate("AppQueryHandler", "Toggle");
    static const auto tr_prop = QCoreApplication::translate("AppQueryHandler", "Property of the extension '%1' [%2]");

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
