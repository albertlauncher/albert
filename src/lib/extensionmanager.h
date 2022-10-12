// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <set>
#include <vector>
#include <memory>
#include "albert/extension.h"

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
