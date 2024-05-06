// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QObject>
#include <QString>
#include <set>
namespace albert {
class ExtensionRegistry;
class PluginInstance;
class PluginLoader;
class PluginMetaData;
class PluginProvider;
}


class Plugin : public QObject
{
    Q_OBJECT

public:

    Plugin(albert::PluginProvider *provider, albert::PluginLoader *loader);

    albert::PluginProvider const * const provider;
    QString path() const;
    const albert::PluginMetaData &metaData() const;
    const QString &id() const;
    bool isUser() const;
    bool isEnabled() const;
    void setEnabled(bool);
    const std::set<Plugin*> &dependencies() const;
    const std::set<Plugin*> &dependees() const;

    enum class State {
        Invalid,
        Unloaded,
        Loaded,
        Busy,
    };

    State state() const;
    const QString &stateInfo() const;
    QString localStateString() const;
    albert::PluginInstance *instance() const;

private:

    QString load() noexcept;
    QString unload() noexcept;

    std::set<Plugin*> transitiveDependencies() const;
    std::set<Plugin*> transitiveDependees() const;

    void setState(State, QString info = {});

    albert::PluginLoader * const loader;
    std::set<Plugin*> dependencies_;
    std::set<Plugin*> dependees_;
    uint load_order;
    bool enabled_;
    QString state_info_;
    State state_;
    albert::PluginInstance *instance_;

    friend class PluginRegistry;

signals:

    void stateChanged();
    void enabledChanged();

};
