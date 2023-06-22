// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/util/extensionwatcher.h"
#include <QObject>
#include <map>
#include <vector>
namespace albert {
class PluginProvider;
class PluginLoader;
}

class PluginRegistry : public QObject, public albert::ExtensionWatcher<albert::PluginProvider>
{
public:
    PluginRegistry(albert::ExtensionRegistry&registry);
    std::vector<const albert::PluginLoader*> plugins() const;
    bool isEnabled(const QString &id) const;
    void enable(const QString &id, bool enable = true);
    void load(const QString &id, bool load = true);

protected:
    void onAdd(albert::PluginProvider*) override;
    void onRem(albert::PluginProvider*) override;

private:
    std::map<QString, albert::PluginLoader*> plugins_;

Q_OBJECT signals:
    void pluginsChanged();
    void pluginStateChanged(const QString &id);

};
