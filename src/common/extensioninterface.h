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
#define ALBERT_EXTENSION_IID "org.manuelschneid3r.albert.extensioninterface"

#include <QtPlugin>
#include <QString>
#include <QIcon>

class Query;
class QueryResult;

class GenericPluginInterface{
public:
    GenericPluginInterface() {}
    virtual ~GenericPluginInterface() {}

    virtual QWidget* widget() {return nullptr;}
    virtual void     initialize() {}
    virtual void     finalize() {}
};

class ExtensionInterface : public GenericPluginInterface
{
public:
    ExtensionInterface() {}
    virtual ~ExtensionInterface() {}

    virtual void     setupSession() = 0;
    virtual void     teardownSession() = 0;
    virtual void     handleQuery(Query *q) = 0;

    virtual const QIcon  &icon     (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) = 0;
    virtual void         action    (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) = 0;
    virtual QString      titleText (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const = 0;
    virtual QString      infoText  (const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const = 0;
    virtual QString      actionText(const Query&, const QueryResult&, Qt::KeyboardModifiers mods) const = 0;
};
Q_DECLARE_INTERFACE(ExtensionInterface, ALBERT_EXTENSION_IID)
