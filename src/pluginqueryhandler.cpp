// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include <QMessageBox>
#include <QWidget>
using namespace albert;
using namespace std;

PluginQueryHandler::PluginQueryHandler(PluginRegistry &plugin_registry) : plugin_registry_(plugin_registry)
{
    QObject::connect(&plugin_registry_, &PluginRegistry::pluginsChanged, &plugin_registry_, [this](){ updateIndexItems(); });
}

QString PluginQueryHandler::id() const { return QStringLiteral("pluginregistry"); }

QString PluginQueryHandler::name() const { return QStringLiteral("Plugins"); }

QString PluginQueryHandler::description() const { return QStringLiteral("Manage plugins"); }

QString PluginQueryHandler::defaultTrigger() const { return QStringLiteral("plugin "); }

class PluginItem : public Item
{
    PluginRegistry &plugin_registry_;
    PluginLoader &loader_;
public:

    PluginItem(PluginRegistry &plugin_registry, PluginLoader &loader):
        plugin_registry_(plugin_registry), loader_(loader) {}

    QString id() const override { return loader_.metaData().id; }

    QString text() const override {
        return QString("%1 plugin (%2)").arg(loader_.metaData().name, loader_.metaData().id);
    }

    QString subtext() const override {
        QString enabled = plugin_registry_.isEnabled(id()) ? "Enabled" : "Disabled";
        QString state;
        if (loader_.state() == PluginState::Loaded)
            state = "Loaded";
        else if (loader_.stateInfo().isEmpty())
            state = "Unloaded";
        else
            state = QString("ERROR: %1").arg(loader_.stateInfo());

        return QString("Config: %1, State: %2").arg(enabled, state);
    }

    QStringList iconUrls() const override { return {QStringLiteral("gen:?&text=🧩")}; }

    vector<Action> actions() const override {
        vector<Action> actions;


        if (plugin_registry_.isEnabled(id()))
            actions.emplace_back(
                "disable", "Disable plugin", [this]() { plugin_registry_.enable(id(), false); }
            );
        else
            actions.emplace_back(
                "enable", "Enable plugin", [this]() { plugin_registry_.enable(id(), true); }
            );


        if (loader_.state() == PluginState::Loaded){

            if (loader_.metaData().load_type == LoadType::User) {

                actions.emplace_back(
                    "unload", "Unload plugin", [this](){ plugin_registry_.load(id(), false); }
                );

                actions.emplace_back(
                    "reload", "Reload plugin",
                    [this](){ plugin_registry_.load(id(), false); plugin_registry_.load(id()); }
                );
            }

        } else  // by contract only unloaded
            actions.emplace_back("load", "Load plugin", [this](){ plugin_registry_.load(id()); });

        actions.emplace_back("settings", "Open settings", [this](){ showSettings(id()); });

        return actions;
    }
};

void PluginQueryHandler::updateIndexItems()
{
    vector<IndexItem> items;
    for (auto &[id, loader] : plugin_registry_.plugins()){
        auto item = make_shared<PluginItem>(plugin_registry_, *loader);
        items.emplace_back(item, id);
        items.emplace_back(item, loader->metaData().name);
    }
    setIndexItems(::move(items));
}
