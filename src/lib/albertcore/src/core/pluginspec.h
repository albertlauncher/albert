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

class PluginSpec final : public QObject
{
    Q_OBJECT

public:

    enum class State { Loaded, NotLoaded, Error };

    PluginSpec(const QString &path);

    QString path() const;
    bool load();
    bool unload();
    QObject *instance();
    State state() { return state_; }
    QString lastError() const;
    QString iid() const;
    QJsonValue metadata(const QString & key) const;

    // Mandatory metadata
    QString id() const;
    QString name() const;
    QString version() const;
    QString author() const;
    QStringList dependencies() const;
    bool enabledByDefault() const;

private:

    QPluginLoader loader_;
    State state_;
    QString lastError_;

signals:

    void pluginLoaded(PluginSpec*);
    void pluginAboutToUnload(PluginSpec*);

};

}




