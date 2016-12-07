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
#include "core_globals.h"

class AbstractExtension;

class EXPORT_CORE AbstractExtensionLoader
{
public:

    enum class State {
        Loaded,
        NotLoaded,
        Error
    };

    AbstractExtensionLoader() : state_(State::NotLoaded) {}
    virtual ~AbstractExtensionLoader() {}

    virtual State state() { return state_; }
    virtual bool load() = 0;
    virtual bool unload() = 0;
    virtual QString lastError() const = 0;
    virtual AbstractExtension* instance() = 0;
    virtual QString path() const = 0;
    virtual QString type() const = 0;
    // Metadata
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString author() const = 0;
    virtual QStringList dependencies() const { return QStringList(); }

protected:

    State state_;

};
