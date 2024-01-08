// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/pluginprovider/pluginprovider.h"
#include "albert/logging.h"
#include "pluginloaderprivate.h"
#include "pluginregistry.h"
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
using namespace albert;
using namespace std;


PluginRegistry::PluginRegistry(ExtensionRegistry &registry)
    : albert::ExtensionWatcher<PluginProvider>(&registry), extension_registry(registry) {}

PluginRegistry::~PluginRegistry() = default; // private PluginLoader dtor

const map<QString, PluginLoader*> &PluginRegistry::plugins() const { return registered_plugins_; }

bool PluginRegistry::isEnabled(const QString &id) const
{ return albert::settings()->value(QString("%1/enabled").arg(id), false).toBool(); }

void PluginRegistry::enable(const QString &id, bool enable)
{
    try {
        auto *loader = registered_plugins_.at(id);
        albert::settings()->setValue(QString("%1/enabled").arg(id), enable);
        emit enabledChanged(id);

        if (enable && loader->state() == PluginState::Unloaded )
        {
            if (auto err = loader->d->load(&extension_registry); !err.isNull())
            {
                auto msg = tr("Failed loading plugin '%1': %2").arg(id, err);
                WARN << msg;
                QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
            }
        }
        else if (!enable && loader->state() == PluginState::Loaded)
        {
            switch (loader->metaData().load_type){
            case albert::LoadType::User:
            {
                if (auto err = loader->d->unload(&extension_registry); !err.isNull())
                {
                    auto msg = tr("Failed unloading plugin '%1': %2").arg(id, err);
                    WARN << msg;
                    QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
                }
                break;
            }
            case albert::LoadType::Frontend:
            {
                auto msg = tr("Frontend plugins cannot be unloaded: '%1'").arg(id);
                WARN << msg;
                QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
                break;
            }
            case albert::LoadType::NoUnload:
            {
                QMessageBox msgBox(QMessageBox::Question, qApp->applicationDisplayName(),
                                   tr("Unloading the plugin '%1' requires restarting the app. "
                                      "Do you want to restart Albert?").arg(id),
                                   QMessageBox::Yes | QMessageBox::No);
                if (msgBox.exec() == QMessageBox::Yes)
                    restart();
                break;
            }

            default:
                break;
            }
        }
    }
    catch (const out_of_range&)
    {
        auto msg = tr("Plugin '%1' does not exist.").arg(id);
        WARN << msg;
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
    }
}

void PluginRegistry::load(const QString &id, bool load)
{
    try
    {
        auto *loader = registered_plugins_.at(id);
        if (load)
        {
            if (auto err = loader->d->load(&extension_registry); !err.isNull())
            {
                auto msg = tr("Failed loading plugin '%1': %2").arg(id, err);
                WARN << msg;
                QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
            }
        }
        else
        {
            if (auto err = loader->d->unload(&extension_registry); !err.isNull())
            {
                auto msg = tr("Failed unloading plugin '%1': %2").arg(id, err);
                WARN << msg;
                QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
            }
        }
    }
    catch (const out_of_range&)
    {
        auto msg = tr("Plugin '%1' does not exist.").arg(id);
        WARN << msg;
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
    }
}

void PluginRegistry::onAdd(PluginProvider *pp)
{
    const auto &plugins = plugins_.emplace(pp, pp->plugins()).first->second;

    for (auto &loader : plugins){
        auto id = loader->metaData().id;

        // Register plugins enforcing unique ids
        if (const auto &[it, success] = registered_plugins_.emplace(id, loader); success){

            // Load if is enabled user plugin
            if (isEnabled(id)
                && (loader->metaData().load_type ==  LoadType::User
                    || loader->metaData().load_type ==  LoadType::NoUnload))
                load(id);

        } else
            INFO << "Plugin" << id << "shadowed:" << loader->path;
    }

    emit pluginsChanged();
}

void PluginRegistry::onRem(PluginProvider *pp)
{
    for (auto it = registered_plugins_.begin(); it != registered_plugins_.end();){
        const auto &[id, loader] = *it;

        if (&loader->provider() == pp){
            if (loader->state() == PluginState::Loaded)
                load(id, false);
            it = registered_plugins_.erase(it);
        } else
            ++it;
    }

    emit pluginsChanged();
}
