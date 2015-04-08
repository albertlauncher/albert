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
#include <QtPlugin>
#include <QString>
#include <QIcon>
#include <memory>
using std::shared_ptr;

#define ALBERT_EXTENSION_IID        "org.manuelschneid3r.albert.extensioninterface"
#define ALBERT_LANGUAGE_BINDING_IID "org.manuelschneid3r.albert.languagebindinginterface"

class Query;

/** ***************************************************************************/
class GenericPluginInterface
{
public:
    GenericPluginInterface() {}
    virtual ~GenericPluginInterface() {}

    virtual QWidget* widget() {return nullptr;}
    virtual void     initialize() {}
    virtual void     finalize() {}
};

/** ***************************************************************************/
class ExtensionInterface : public GenericPluginInterface
{
public:
    ExtensionInterface() {}
    virtual ~ExtensionInterface() {}

    virtual void     setupSession() {}
    virtual void     teardownSession() {}
    virtual void     handleQuery(Query *q) = 0;
};
Q_DECLARE_INTERFACE(ExtensionInterface, ALBERT_EXTENSION_IID)

/** ***************************************************************************/
class ItemInterface
{
public:
    ItemInterface() {}
    virtual ~ItemInterface(){}

    virtual void         action    (const Query &, Qt::KeyboardModifiers) {}
    virtual QString      actionText(const Query &, Qt::KeyboardModifiers) const = 0;
    virtual QString      titleText (const Query &) const = 0;
    virtual QString      infoText  (const Query &) const = 0;
    virtual const QIcon  &icon     () const = 0;
    virtual uint         usage     () const { return 0;}
};
typedef shared_ptr<ItemInterface> SharedItemPtr;
typedef QList<SharedItemPtr> SharedItemPtrList;

/** ***************************************************************************/
class LanguageBindingInterface : public GenericPluginInterface
{
public:
    LanguageBindingInterface() {}
    virtual ~LanguageBindingInterface() {}
};
Q_DECLARE_INTERFACE(LanguageBindingInterface, ALBERT_LANGUAGE_BINDING_IID)
