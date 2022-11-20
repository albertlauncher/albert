// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/pluginprovider.h"
#include "albert/extensionregistry.h"
#include "albert/plugin.h"
#include <set>
using namespace std;
using namespace albert;

extern PluginSpec *current_spec_in_construction;
extern ExtensionRegistry *extension_registry;

struct Plugin::Private
{
    PluginSpec &spec = *current_spec_in_construction;
    ExtensionRegistry &registry = *extension_registry;
    set<Extension*> registered_extensions;
};

Plugin::Plugin(): d(std::make_unique<Private>()) {}

Plugin::~Plugin()
{
    for (auto *e : d->registered_extensions)
        d->registry.remove(e);
}

QString Plugin::id() const
{
    return d->spec.id;
}

QString Plugin::name() const
{
    return d->spec.name;
}

QString Plugin::description() const
{
    return d->spec.description;
}

PluginSpec &albert::Plugin::spec()
{
    return d->spec;
}

void albert::Plugin::addExtension(Extension *e)
{
    d->registered_extensions.insert(e);
    d->registry.add(e);
}

ExtensionRegistry &albert::Plugin::registry()
{
    return d->registry;
}

