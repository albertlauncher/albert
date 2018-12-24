// Copyright (C) 2014-2018 Manuel Schneider

#include <set>
#include "albert/extension.h"
#include "extensionmanager.h"
using namespace std;


Core::ExtensionManager *Core::Extension::extensionManager = nullptr;

struct Core::Private {
    set<QueryHandler*> registeredQueryHandlers;
    set<FallbackProvider*> registeredFallbackProviders;
};


/**************************************************************************************************/
Core::Extension::Extension(const QString &id) : Plugin(id), d(new Private) {

}


/**************************************************************************************************/
Core::Extension::~Extension() {
    // If the extensin did it not by itself unregister all the remaining handlers
    for (auto ptr : d->registeredQueryHandlers)
        unregisterQueryHandler(ptr);
    for (auto ptr : d->registeredFallbackProviders)
        unregisterFallbackProvider(ptr);
}


/**************************************************************************************************/
void Core::Extension::registerQueryHandler(Core::QueryHandler *object) {
    d->registeredQueryHandlers.insert(object);
    extensionManager->registerQueryHandler(object);
}


/**************************************************************************************************/
void Core::Extension::unregisterQueryHandler(Core::QueryHandler *object) {
    extensionManager->unregisterQueryHandler(object);
}


/**************************************************************************************************/
void Core::Extension::registerFallbackProvider(Core::FallbackProvider *object) {
    d->registeredFallbackProviders.insert(object);
    extensionManager->registerFallbackProvider(object);
}


/**************************************************************************************************/
void Core::Extension::unregisterFallbackProvider(Core::FallbackProvider *object) {
    extensionManager->unregisterFallbackProvider(object);
}
