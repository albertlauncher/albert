// Copyright (c) 2022 Manuel Schneider

#include "albert/extensionregistry.h"
#include "albert/extension.h"
using namespace std;
using namespace albert;

void ExtensionRegistry::add(Extension *e)
{
    const auto&[it, success] = extensions_.emplace(e->id(), e);
    if (success)
        emit added(e);
    else
        qFatal("Duplicate extension registration: %s", qPrintable(e->id()));
}

void ExtensionRegistry::remove(Extension *e)
{
    if (extensions_.erase(e->id()))
            emit removed(e);
    else
        qFatal("Extension removed more than once: %s", qPrintable(e->id()));
}

const map<QString,Extension*> &ExtensionRegistry::extensions()
{
    return extensions_;
}
