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
#include <QIcon>
#include <QString>
#include <QMimeType>
#include <memory>
#include <QMetaType>

/** ****************************************************************************/
struct Object {
    virtual ~Object() {}
    virtual QString name() = 0;
    virtual QString description() = 0;
    virtual QStringList alises() = 0;
    virtual QIcon icon() = 0;
    virtual uint usage() {return 0;}
};

/** ****************************************************************************/
class Action;
struct AlbertObject : public Object {
    virtual ~AlbertObject(){}
    virtual QList<std::shared_ptr<Action>> actions() = 0;
};
typedef QSharedPointer<Object> SharedObject;


/** ****************************************************************************/
struct Action : public Object {
    virtual ~Action(){}
    virtual AlbertObject* object() const = 0;
    virtual void execute() const = 0;
};
