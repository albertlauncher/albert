// Copyright (c) 2022 Manuel Schneider

#include "albert/extension.h"
#include "albert/extensionregistry.h"
#include "albert/logging.h"
using namespace std;
using namespace albert;

void ExtensionRegistry::add(Extension *e)
{
    const auto&[it, success] = extensions_.emplace(e->id(), e);
    if (success)
        emit added(e);
    else
        CRIT << "Extension registered more than once:" << e->id();
}

void ExtensionRegistry::remove(Extension *e)
{
    if (extensions_.erase(e->id()))
        emit removed(e);
    else
        CRIT << "Removed extension that has not been registered before:" << e->id();
}

const map<QString,Extension*> &ExtensionRegistry::extensions()
{
    return extensions_;
}
