// Copyright (c) 2024 Manuel Schneider

#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "pluginconfigqueryhandler.h"
#include "pluginregistry.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
using namespace albert;
using namespace std;

const QStringList PluginConfigQueryHandler::icon_urls{QStringLiteral("gen:?text=⚙️")};

PluginConfigQueryHandler::PluginConfigQueryHandler(PluginRegistry&r):
    plugin_registry_(r)
{}

QString PluginConfigQueryHandler::id() const
{ return QStringLiteral("pluginconfig"); }

QString PluginConfigQueryHandler::name() const
{ return tr("Plugin config"); }

QString PluginConfigQueryHandler::description() const
{ return tr("Quick access to plugin settings"); }

QString PluginConfigQueryHandler::defaultTrigger() const
{ return QStringLiteral("conf "); }

vector<RankItem> PluginConfigQueryHandler::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> rank_items;

    static const auto tr_enabled = tr("Enabled");
    static const auto tr_disabled = tr("Disabled");
    static const auto tr_toggle = tr("Toggle");
    static const auto tr_prop = tr("Property of the plugin '%1' [%2]");

    for (auto &[id, plugin] : plugin_registry_.plugins())
    {
        if (auto *qobject = dynamic_cast<QObject*>(plugin.instance()); qobject)
        {
            auto *metaObj = qobject->metaObject();
            for (int i = metaObj->propertyOffset(); i < metaObj->propertyCount(); ++i)
            {
                auto metaProp = metaObj->property(i);
                QString metaPropName{metaProp.name()};
                if (metaPropName.contains(query->string(), Qt::CaseInsensitive))
                {
                    if(metaProp.isUser())
                    {
                        if (metaProp.typeId() == QMetaType::fromType<bool>().id())
                        {
                            rank_items.emplace_back(
                                StandardItem::make(
                                    QString("%1_%2").arg(id, metaPropName),
                                    QString("%1: %2").arg(metaPropName,
                                                          metaProp.read(qobject).value<bool>()
                                                              ? tr_enabled : tr_disabled),
                                    tr_prop.arg(plugin.instance()->name(), metaProp.typeName()),
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
    }

    return rank_items;
}

