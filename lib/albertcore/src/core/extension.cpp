// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <set>
#include "extension.h"
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
