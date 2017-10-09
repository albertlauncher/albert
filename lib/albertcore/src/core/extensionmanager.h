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

#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <set>
#include <vector>
#include <memory>
#include "extension.h"

namespace Core {

class Extension;
class QueryHandler;
class FallbackProvider;
class PluginSpec;
class ExtensionManagerPrivate;

class ExtensionManager final : public QObject
{
    Q_OBJECT

public:

    ExtensionManager(QStringList pluginroots);
    ~ExtensionManager();

    const std::vector<std::unique_ptr<PluginSpec>> & extensionSpecs() const;

    void reloadExtensions();

    void enableExtension(const std::unique_ptr<PluginSpec> &);
    void disableExtension(const std::unique_ptr<PluginSpec> &);
    bool extensionIsEnabled(const std::unique_ptr<PluginSpec> &);

    void registerQueryHandler(QueryHandler*);
    void unregisterQueryHandler(QueryHandler*);
    const std::set<QueryHandler *> &queryHandlers();

    void registerFallbackProvider(FallbackProvider*);
    void unregisterFallbackProvider(FallbackProvider*);
    const std::set<FallbackProvider *> &fallbackProviders();

private:

    void loadExtension(const std::unique_ptr<PluginSpec> &);
    void unloadExtension(const std::unique_ptr<PluginSpec> &);

    std::unique_ptr<ExtensionManagerPrivate> d;

signals:

    void queryHandlerRegistered(QueryHandler*);
    void queryHandlerUnregistered(QueryHandler*);

    void fallbackProviderRegistered(FallbackProvider*);
    void fallbackProviderUnregistered(FallbackProvider*);

};

}
