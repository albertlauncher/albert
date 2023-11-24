// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extensionregistry.h"
#include "albert/logging.h"
#include "plugininstanceprivate.h"
#include <QCoreApplication>
#include <chrono>
using namespace albert;
using namespace std;
using namespace chrono;

class PluginLoader::PluginLoaderPrivate
{
public:

    PluginLoader *q;
    QString state_info{};
    PluginState state{PluginState::Unloaded};

    PluginLoaderPrivate(PluginLoader *l) : q(l)
    {
    }

    void setState(PluginState s, QString info = {})
    {
        state = s;
        state_info = info;
        emit q->stateChanged();
    }

    QString load(ExtensionRegistry *registry)
    {
        switch (state) {
            case PluginState::Loaded:
                return QStringLiteral("Plugin is already loaded.");
            case PluginState::Busy:
                return QStringLiteral("Plugin is currently busy.");
            case PluginState::Unloaded:
            {
                QCoreApplication::processEvents();

                setState(PluginState::Busy, QStringLiteral("Loading…"));
                QStringList errors;
                auto start = system_clock::now();

                try {
                    PluginInstancePrivate::in_construction = q;
                    if (auto err = q->load(); err.isEmpty()){
                        if (auto *p_instance = q->instance()){

                            p_instance->initialize(registry);
                            for (auto *e : p_instance->extensions())
                                registry->add(e);

                            setState(PluginState::Loaded);

                            DEBG << QStringLiteral("[%1 ms] spent loading plugin '%2'")
                                        .arg(duration_cast<milliseconds>(system_clock::now()-start).count())
                                        .arg(q->metaData().id);

                            return {};

                        } else
                            errors << "Plugin laoded successfully but returned nullptr instance.";
                    } else
                        errors << err;

                    if (auto err = q->unload(); !err.isNull())
                        errors << err;

                } catch (const exception& e) {
                    errors << e.what();
                } catch (...) {
                    errors << "Unknown exception while loading plugin.";
                }

                auto err_str = errors.join("\n");
                setState(PluginState::Unloaded, err_str);
                return err_str;
            }
        }
        return {};
    }

    QString unload(ExtensionRegistry *registry)
    {
        switch (state) {
            case PluginState::Unloaded:
                return QStringLiteral("Plugin is not loaded.");
            case PluginState::Busy:
                return QStringLiteral("Plugin is currently busy.");
            case PluginState::Loaded:
            {
                setState(PluginState::Busy, QStringLiteral("Loading…"));
                auto start = system_clock::now();

                QStringList errors;
                if (auto *p_instance = q->instance()){

                    for (auto *e : p_instance->extensions())
                        registry->remove(e);

                    try {
                        p_instance->finalize(registry);

                        if (auto err = q->unload(); !err.isNull())
                            errors << err;

                    } catch (const exception& e) {
                        errors << e.what();
                    } catch (...) {
                        errors << "Unknown exception while unloading plugin.";
                    }
                } else
                    errors << "Plugin laoded but returned nullptr instance.";

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
};
