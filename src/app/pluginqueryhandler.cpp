// Copyright (c) 2023-2025 Manuel Schneider

#include "albert.h"
#include "iconutil.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include <QWidget>
using enum Plugin::State;
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;


class PluginItem : public Item
{
    PluginRegistry &plugin_registry_;
    const Plugin &plugin_;
public:

    PluginItem(PluginRegistry &plugin_registry, const Plugin &plugin):
        plugin_registry_(plugin_registry), plugin_(plugin) {}

    QString id() const override { return plugin_.id; }

    QString text() const override
    { return QString("%1 (%2)").arg(plugin_.metadata.name, plugin_.id); }

    QString subtext() const override
    {
        QString state;
        if (plugin_.state == Loaded)
        {
            static const auto tr_loaded = PluginQueryHandler::tr("Loaded");
            state = tr_loaded;
        }
        else
        {
            static const auto tr_unloaded = PluginQueryHandler::tr("Unloaded");
            state = tr_unloaded;
        }

        if (!plugin_.state_info.isEmpty())
            state.append(QString(" (%1)").arg(plugin_.state_info));

        static const auto tr_config = PluginQueryHandler::tr("Configuration");
        static const auto tr_enabled = PluginQueryHandler::tr("Enabled");
        static const auto tr_disabled = PluginQueryHandler::tr("Disabled");
        static const auto tr_state = PluginQueryHandler::tr("State");
        return QString("%1: %2, %3: %4")
            .arg(tr_config, plugin_.enabled ? tr_enabled : tr_disabled,
                 tr_state, state);
    }

    QString inputActionText() const override
    { return plugin_.metadata.name; }

    unique_ptr<Icon> icon() const override
    {
        if(!plugin_.enabled)
            return makeGraphemeIcon(u"🧩"_s);
        else if (plugin_.state == Loaded)
            return makeComposedIcon(makeGraphemeIcon(u"🧩"_s), makeGraphemeIcon(u"✅"_s), 1.0, 0.5);
        else if (plugin_.state_info.isEmpty())
            return makeComposedIcon(makeGraphemeIcon(u"🧩"_s), makeGraphemeIcon(u"⏳"_s), 1.0, 0.5);
        else
            return makeComposedIcon(makeGraphemeIcon(u"🧩"_s), makeGraphemeIcon(u"⚠️"_s), 1.0, 0.5);
    }

    vector<Action> actions() const override
    {
        vector<Action> actions;

        actions.emplace_back(
            "settings",
            PluginQueryHandler::tr("Open settings"),
            [this] { showSettings(id()); }
            );

        actions.emplace_back(
            plugin_.enabled ? "disable" : "enable",
            plugin_.enabled ? PluginQueryHandler::tr("Disable")
                            : PluginQueryHandler::tr("Enable"),
            [this] { plugin_registry_.setEnabledWithUserConfirmation(plugin_.id, !plugin_.enabled); }
        );

        return actions;
    }
};


PluginQueryHandler::PluginQueryHandler(PluginRegistry &plugin_registry) : plugin_registry_(plugin_registry)
{
    QObject::connect(&plugin_registry_, &PluginRegistry::pluginsChanged,
                     &plugin_registry_, [this] { setIndexItems({}); updateIndexItems(); });
}

QString PluginQueryHandler::id() const { return u"pluginregistry"_s; }

QString PluginQueryHandler::name() const { return tr("Plugins"); }

QString PluginQueryHandler::description() const { return tr("Manage plugins"); }

QString PluginQueryHandler::defaultTrigger() const { return u"plugin "_s; }

void PluginQueryHandler::updateIndexItems()
{
    vector<IndexItem> items;
    for (auto &[id, plugin] : plugin_registry_.plugins()){
        auto item = make_shared<PluginItem>(plugin_registry_, plugin);
        items.emplace_back(item, id);
        items.emplace_back(item, plugin.metadata.name);
    }
    setIndexItems(::move(items));
}
