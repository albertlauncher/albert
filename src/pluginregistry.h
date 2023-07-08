// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
#include "albert/extensionwatcher.h"
#include <QObject>
#include <map>
#include <vector>
namespace albert {
class PluginProvider;
class PluginLoader;
}

class PluginRegistry : public QObject, public albert::ExtensionWatcher<albert::PluginProvider>
{
    Q_OBJECT
public:
    PluginRegistry(albert::ExtensionRegistry&);
    ~PluginRegistry();

    std::map<QString, albert::PluginLoader*> plugins() const;

    bool isEnabled(const QString &id) const;
    void enable(const QString &id, bool enable = true);
    QString load(const QString &id, bool load = true);

    albert::ExtensionRegistry &extension_registry;

protected:
    void onAdd(albert::PluginProvider*) override;
    void onRem(albert::PluginProvider*) override;

private:
    std::map<QString, albert::PluginLoader*> registered_plugins_;
    std::map<albert::PluginProvider*, std::vector<albert::PluginLoader*>> plugins_;

signals:
    void pluginsChanged();

};
