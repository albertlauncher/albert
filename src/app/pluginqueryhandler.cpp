// Copyright (c) 2023-2024 Manuel Schneider

#include "pluginmetadata.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include "util.h"
#include <QMessageBox>
#include <QWidget>
using namespace albert;
using namespace std;


class PluginItem : public Item
{
    PluginRegistry &plugin_registry_;
    const Plugin &plugin_;
public:

    PluginItem(PluginRegistry &plugin_registry, const Plugin &plugin):
        plugin_registry_(plugin_registry), plugin_(plugin) {}

    QString id() const override { return plugin_.id(); }

    QString text() const override
    { return QString("%1 (%2)").arg(plugin_.metaData().name, plugin_.id()); }

    QString subtext() const override
    {
        QString state;
        if (plugin_.state() == Plugin::State::Loaded)
        {
            static const auto tr_loaded = PluginQueryHandler::tr("Loaded");
            state = tr_loaded;
        }
        else
        {
            static const auto tr_unloaded = PluginQueryHandler::tr("Unloaded");
            state = tr_unloaded;
        }

        if (!plugin_.stateInfo().isEmpty())
            state.append(QString(" (%1)").arg(plugin_.stateInfo()));

        static const auto tr_config = PluginQueryHandler::tr("Configuration");
        static const auto tr_enabled = PluginQueryHandler::tr("Enabled");
        static const auto tr_disabled = PluginQueryHandler::tr("Disabled");
        static const auto tr_state = PluginQueryHandler::tr("State");
        return QString("%1: %2, %3: %4")
            .arg(tr_config, plugin_.isEnabled() ? tr_enabled : tr_disabled,
                 tr_state, state);
    }

    QString inputActionText() const override
    { return plugin_.metaData().name; }

    QStringList iconUrls() const override
    {
        if(!plugin_.isEnabled())
            return {QStringLiteral("gen:?&text=🧩&fontscalar=0.7")};
        else if (plugin_.state() == Plugin::State::Loaded)
            return {QStringLiteral("gen:?&text=🧩&fontscalar=0.7&background=#4000A000")};
        else if (plugin_.stateInfo().isEmpty())
            return {QStringLiteral("gen:?&text=🧩&fontscalar=0.7&background=#4000A0A0")};
        else
            return {QStringLiteral("gen:?&text=🧩&fontscalar=0.7&background=#40FF0000")};
    }

    vector<Action> actions() const override
    {
        vector<Action> actions;

        static const auto tr_open_settings = PluginQueryHandler::tr("Open settings");
        actions.emplace_back(
            "settings",
            tr_open_settings,
            [this](){ showSettings(id()); }
            );

        if (plugin_.isEnabled())
        {
            static const auto tr_disable = PluginQueryHandler::tr("Disable");
            actions.emplace_back(
                "disable",
                tr_disable,
                [this]() { plugin_registry_.disable(plugin_.id()); }
                );
        }
        else
        {
            static const auto tr_enable = PluginQueryHandler::tr("Enable");
            actions.emplace_back(
                "enable",
                tr_enable,
                [this]() { plugin_registry_.enable(plugin_.id()); }
                );
        }

        if (plugin_.state() == Plugin::State::Loaded)
        {
            if (plugin_.isUser())
            {
                static const auto tr_unload = PluginQueryHandler::tr("Unload");
                actions.emplace_back(
                    "unload",
                    tr_unload,
                    [this](){ plugin_registry_.unload(id()); }
                    );

                static const auto tr_reload = PluginQueryHandler::tr("Reload");
                actions.emplace_back(
                    "reload",
                    tr_reload,
                    [this](){ plugin_registry_.unload(id()); plugin_registry_.load(id()); }
                    );
            }
        }
        else  // by contract only unloaded
        {
            static const auto tr_load = PluginQueryHandler::tr("Load");
            actions.emplace_back(
                "load",
                tr_load,
                [this](){ plugin_registry_.load(id()); }
                );
        }

        return actions;
    }
};


PluginQueryHandler::PluginQueryHandler(PluginRegistry &plugin_registry) : plugin_registry_(plugin_registry)
{
    QObject::connect(&plugin_registry_, &PluginRegistry::pluginsChanged,
                     &plugin_registry_, [this](){ updateIndexItems(); });
}

QString PluginQueryHandler::id() const
{ return QStringLiteral("pluginregistry"); }

QString PluginQueryHandler::name() const
{ return tr("Plugins"); }

QString PluginQueryHandler::description() const
{ return tr("Manage plugins"); }

QString PluginQueryHandler::defaultTrigger() const
{ return QStringLiteral("plugin "); }

void PluginQueryHandler::updateIndexItems()
{
    vector<IndexItem> items;
    for (auto &[id, plugin] : plugin_registry_.plugins()){
        auto item = make_shared<PluginItem>(plugin_registry_, plugin);
        items.emplace_back(item, id);
        items.emplace_back(item, plugin.metaData().name);
    }
    setIndexItems(::move(items));
}
