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




