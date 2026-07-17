// Copyright (c) 2026-2026 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "frontendregistry.h"
#include "logging.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "qtpluginprovider.h"
#include <QEventLoop>
#include <QSettings>
#include <QTimer>
#include <expected>
using namespace Qt::StringLiterals;
using namespace albert::detail;
using namespace albert;
using namespace std;

namespace {
static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
}

static expected<Frontend*, QString> loadFrontendPlugin(PluginLoader *loader)
{
    // Blocking load
    QEventLoop loop;

    QObject::connect(loader, &PluginLoader::finished, &loop, [&](QString info) {
        if (!info.isEmpty())
            DEBG << info;
        loop.quit();
    });

    QTimer::singleShot(0, loader, [loader]{ loader->load(); });

    loop.exec();

    QObject::connect(loader->instance(), &PluginInstance::initialized,
                     &loop, [&] { loop.quit(); });

    QTimer::singleShot(0, loader, [loader]{ loader->instance()->initialize(); });

    loop.exec();

    if (auto frontend = dynamic_cast<Frontend*>(loader->instance()))
        return frontend;
    else
        return unexpected(
            QString("Failed casting plugin instance to albert::Frontend: %1")
                .arg(loader->metadata().id));
}

class FrontendRegistry::Private
{
public:
    const QtPluginProvider &plugin_provider;
    Frontend &frontend;

    Private(const QSettings &settings, const QtPluginProvider &plugin_provider) :
        plugin_provider(plugin_provider),
        frontend(loadFrontend(settings.value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString()))
    {}

    vector<PluginLoader*> frontendPlugins() const
    {
        vector<PluginLoader*> loaders;
        for (const auto &loader : plugin_provider.plugins())
            if (loader->metadata().load_type == PluginMetadata::LoadType::Frontend)
                loaders.emplace_back(loader);
        return loaders;
    }

    Frontend &loadFrontend(const QString &id) const
    {
        vector<PluginLoader*> loaders = frontendPlugins();
        DEBG << u"Try loading the configured frontend '%1'."_s.arg(id);

        if (auto it = ranges::find(loaders, id, [&](auto *loader){ return loader->metadata().id; });
            it != loaders.end())
            if (auto exp_frontend = loadFrontendPlugin(*it))
                return **exp_frontend;
            else
            {
                WARN << u"Loading configured frontend '%1' failed: %2."_s.arg(id, exp_frontend.error());
                loaders.erase(it);
            }
        else
            WARN << u"Configured frontend plugin '%1' does not exist."_s.arg(id);

        for (auto &loader : loaders)
        {
            WARN << u"Try loading '%1'."_s.arg(loader->metadata().id);

            if (auto exp_frontend = loadFrontendPlugin(loader))
            {
                INFO << u"Using '%1' as fallback."_s.arg(loader->metadata().id);
                return **exp_frontend;
            }
            else
                WARN << u"Failed loading '%1'."_s.arg(loader->metadata().id);
        }

        qFatal("Could not load any frontend.");
    }
};

FrontendRegistry::FrontendRegistry(const QSettings &s, const QtPluginProvider &pp) :
    d(make_unique<Private>(s, pp))
{}

FrontendRegistry::~FrontendRegistry() { const_cast<PluginLoader&>(d->frontend.loader()).unload(); }

vector<PluginLoader *> FrontendRegistry::availableFrontends() { return d->frontendPlugins(); }

Frontend &FrontendRegistry::frontend() { return d->frontend; }

void FrontendRegistry::setFrontend(unsigned int i)
{ app().settings()->setValue(CFG_FRONTEND_ID, d->frontendPlugins().at(i)->metadata().id); }