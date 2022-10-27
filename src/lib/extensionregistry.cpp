// Copyright (c) 2022 Manuel Schneider

#include "extensionregistry.h"
#include "extension.h"

void albert::ExtensionRegistry::add(albert::Extension *e)
{
    const auto&[it, success] = extensions_.emplace(e->id(), e);
    if (success)
            emit added(e);
    else
        qFatal("Duplicate extension registration: %s", qPrintable(e->id()));
}

void albert::ExtensionRegistry::remove(albert::Extension *e)
{
    if (extensions_.erase(e->id()))
            emit removed(e);
    else
        qFatal("Extension removed more than once: %s", qPrintable(e->id()));
}

const std::map<QString,albert::Extension*> &albert::ExtensionRegistry::extensions()
{
    return extensions_;
}