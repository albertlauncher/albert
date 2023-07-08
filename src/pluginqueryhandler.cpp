// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
using namespace albert;

PluginQueryHandler::PluginQueryHandler(PluginRegistry &plugin_registry) : plugin_registry_(plugin_registry){}

QString PluginQueryHandler::id() const { return QStringLiteral("pluginregistry"); }

QString PluginQueryHandler::name() const { return QStringLiteral("Plugins"); }

QString PluginQueryHandler::description() const { return QStringLiteral("Manage plugins"); }

QString PluginQueryHandler::defaultTrigger() const { return QStringLiteral("plugin "); }

void PluginQueryHandler::handleTriggerQuery(TriggerQuery *query) const
{
    for (auto &[id, loader] : plugin_registry_.plugins()){

        if (!(id.contains(query->string(), Qt::CaseInsensitive)
              || loader->metaData().name.contains(query->string(), Qt::CaseInsensitive)))
            continue;

        std::vector<Action> actions;
        QString info;

        if (loader->metaData().user){
            if (plugin_registry_.isEnabled(id))
                actions.emplace_back(
                    "disable", "Disable plugin",
                    [this, id=id]() { plugin_registry_.enable(id, false); }
                    );
            else
                actions.emplace_back(
                    "enable", "Enable plugin",
                    [this, id=id]() { plugin_registry_.enable(id); }
                    );

            if (loader->state() == PluginState::Loaded){
                actions.emplace_back(
                    "unload", "Unload plugin",
                    [this, id=id](){ plugin_registry_.load(id, false); }
                    );
                actions.emplace_back(
                    "reload", "Reload plugin",
                    [this, id=id](){ plugin_registry_.load(id, false); plugin_registry_.load(id, true); }
                    );
            }
            else  // by contract only unloaded
                actions.emplace_back(
                    "load", "Load plugin",
                    [this, id=id](){ plugin_registry_.load(id); }
                    );

            QString enabled = plugin_registry_.isEnabled(id) ? "Enabled" : "Disabled";
            QString state;
            if (loader->state() == PluginState::Loaded)
                state = "Loaded";
            else if (loader->stateInfo().isEmpty())
                state = "Unloaded";
            else
                state = QString("ERROR: %1").arg(loader->stateInfo());
            info = QString("Config: %1, State: %2").arg(enabled, state);

            query->add(
                StandardItem::make(
                    id,
                    QString("%1 (%2)").arg(loader->metaData().name, loader->metaData().id),
                    info,
                    {":app_icon"},
                    actions
                    )
                );
        }
    }
}
