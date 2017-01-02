// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QString>
#include <QStringList>
#include <QPluginLoader>
#include "core_globals.h"

namespace Core {

class Extension;

class EXPORT_CORE ExtensionSpec final
{
public:

    enum class State {
        Loaded,
        NotLoaded,
        Error
    };

    ExtensionSpec(QString path) : loader_(path), state_(State::NotLoaded) { }
    ~ExtensionSpec() { }

    State state() { return state_; }
    bool load();
    bool unload();
    QString lastError() const;
    QObject *instance();
    QString path() const;
    QString type() const;
    // Metadata
    QString id() const;
    QString name() const;
    QString version() const;
    QString author() const;
    QStringList dependencies() const;

private:

    QPluginLoader loader_;
    State state_;

};

}




