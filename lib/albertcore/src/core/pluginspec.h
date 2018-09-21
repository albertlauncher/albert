// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonValue>
#include <QPluginLoader>

namespace Core {

class PluginSpec
{
public:

    enum class State : char {
        Loaded,
        NotLoaded,
        Error
    };

    PluginSpec(const QString &path);
    ~PluginSpec();
    PluginSpec(const PluginSpec &other) = delete;
    PluginSpec &operator=(const PluginSpec &other) = delete;

    QString path() const;
    QString iid() const;
    QString id() const;
    QString name() const;
    QString version() const;
    QString author() const;
    QStringList dependencies() const;
    QJsonValue metadata(const QString & key) const;

    bool load();
    void unload();
    State state() const;
    QString lastError() const;

    QObject *instance();

private:

    QPluginLoader loader_;
    QString iid_;
    QString id_;
    QString name_;
    QString version_;
    QString author_;
    QStringList dependencies_;
    QString lastError_;
    State state_;

};

}




