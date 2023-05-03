// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/queryhandler.h"
#include "albert/util/extensionwatcher.h"
#include <set>
#include <QObject>
namespace albert {
class PluginProvider;
class PluginLoader;
}

class PluginRegistry : public QObject,
                       public albert::TriggerQueryHandler,
                       public albert::ExtensionWatcher<albert::PluginProvider>
{
public:
    PluginRegistry(albert::ExtensionRegistry&registry);
    std::vector<const albert::PluginLoader*> plugins() const;
    bool isEnabled(const QString &id) const;
    void enable(const QString &id, bool enable = true);
    void load(const QString &id, bool load = true);

protected:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void onAdd(albert::PluginProvider*) override;
    void onRem(albert::PluginProvider*) override;
    void handleTriggerQuery(TriggerQuery &) const override;

private:
    std::map<QString, albert::PluginLoader*> plugins_;

Q_OBJECT signals:
    void pluginsChanged();
    void pluginStateChanged(const QString &id);

};
