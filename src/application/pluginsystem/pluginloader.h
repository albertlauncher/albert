// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include <QList>
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include <QPluginLoader>


#include <QSettings>
class PluginLoader : public QPluginLoader {

public:
    enum class Status{NotLoaded, Error, Loaded};

    PluginLoader(QString path);
    PluginLoader(const PluginLoader&) = delete;
    ~PluginLoader();

    QObject *instance();
    void load();
    void unload();

    Status  status() const;
    QString IID() const;
    QString id() const;
    QString name() const;
    QString version() const;
    QString platform() const;
    QString group() const;
    QString copyright() const;
    QString description() const;
    QStringList dependencies() const;

private:
    Status _status;
};
