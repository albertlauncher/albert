// Copyright (c) 2024 Manuel Schneider

#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extensionregistry.h"
#include "albert/logging.h"
#include "plugininstanceprivate.h"
#include "pluginloaderprivate.h"
#include <QCoreApplication>
#include <chrono>
using namespace albert;
using namespace std::chrono;
using namespace std;

PluginLoader::PluginLoaderPrivate::PluginLoaderPrivate(PluginLoader *l) : q(l)
{
}

void PluginLoader::PluginLoaderPrivate::setState(PluginState s, QString info)
{
    state = s;
    state_info = info;
    emit q->stateChanged();
}

QString PluginLoader::PluginLoaderPrivate::load(ExtensionRegistry *registry)
{
    switch (state)
    {
    case PluginState::Loaded:
        return QCoreApplication::translate("PluginLoaderPrivate", "Plugin is already loaded.");
    case PluginState::Busy:
        return QCoreApplication::translate("PluginLoaderPrivate", "Plugin is currently busy.");
    case PluginState::Unloaded:
    {
        setState(PluginState::Busy, QCoreApplication::translate("PluginLoaderPrivate", "Loading…"));

        QCoreApplication::processEvents();

        QStringList errors;
        auto start = system_clock::now();

        try
        {
            PluginInstancePrivate::in_construction = q;
            if (auto err = q->load(); err.isEmpty())
            {
                if (auto *p_instance = q->instance())
                {
                    p_instance->initialize(registry);
                    for (auto *e : p_instance->extensions())
                        registry->add(e);

                    setState(PluginState::Loaded);

                    DEBG << QStringLiteral("[%1 ms] spent loading plugin '%2'")
                                .arg(duration_cast<milliseconds>(system_clock::now()-start).count())
                                .arg(q->metaData().id);

                    return {};
                }
                else
                    errors << QCoreApplication::translate(
                        "PluginLoaderPrivate",
                        "Plugin loaded successfully but returned nullptr instance."
                        );
            }
            else
                errors << err;

            if (auto err = q->unload(); !err.isNull())
                errors << err;
        }
        catch (const exception& e)
        {
            errors << e.what();
        }
        catch (...)
        {
            errors << QCoreApplication::translate("PluginLoaderPrivate",
                                                  "Unknown exception while loading plugin.");
        }

        auto err_str = errors.join("\n");
        setState(PluginState::Unloaded, err_str);
        return err_str;
    }
    }
    return {};
}

QString PluginLoader::PluginLoaderPrivate::unload(ExtensionRegistry *registry)
{
    switch (state)
    {
    case PluginState::Unloaded:
        return QCoreApplication::translate("PluginLoaderPrivate", "Plugin is not loaded.");
    case PluginState::Busy:
        return QCoreApplication::translate("PluginLoaderPrivate", "Plugin is currently busy.");
    case PluginState::Loaded:
    {
        setState(PluginState::Busy, QCoreApplication::translate("PluginLoaderPrivate",
                                                                "Unloading…"));
        auto start = system_clock::now();

        QStringList errors;
        auto *p_instance = q->instance();
        if (!p_instance)
            qFatal("Logic error: A loaded plugin must have a valid instance pointer.");

        try
        {
            for (auto *e : p_instance->extensions())
                registry->remove(e);

            p_instance->finalize(registry);

            if (auto err = q->unload(); !err.isNull())
                errors << err;
        }
        catch (const exception& e)
        {
            errors << e.what();
        }
        catch (...)
        {
            errors << QCoreApplication::translate("PluginLoaderPrivate",
                                                  "Unknown exception while unloading plugin.");
        }

        auto err_str = errors.join("\n");
        setState(PluginState::Unloaded, err_str);

        DEBG << QStringLiteral("[%1 ms] spent unloading plugin '%2'")
                    .arg(duration_cast<milliseconds>(system_clock::now()-start).count())
                    .arg(q->metaData().id);

        return err_str;
    }
    }
    return {};
}
