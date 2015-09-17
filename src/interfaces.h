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
#include <QWidget>
class QIcon;
class QString;
class QMimeType;
class QtPlugin;
class Query;



/** ***************************************************************************/
struct INode
{
    virtual ~INode() {}

    virtual QString       name(Query const *) const = 0;
    virtual QString       description(Query const*) const = 0;
    virtual QIcon         icon() const = 0;
    virtual void          activate(Query const *) = 0;
    virtual unsigned int  usage() const = 0;
    virtual INode         *parent() const = 0;
    virtual QList<INode*> children() = 0;
    virtual bool          hasChildren() const = 0;
};



/** ***************************************************************************/
struct RootNode : public INode
{
    virtual ~RootNode() {}
    INode *parent()     const override final {return nullptr;}
    bool  hasChildren() const override final {return true;}
};



/** ***************************************************************************/
struct LeafNode : public INode
{
    virtual ~LeafNode() {}
    QList<INode*> children() override final {return QList<INode*>();}
    bool          hasChildren() const override final {return false;}
};



/** ***************************************************************************/
/** ***************************************************************************/



/** ***************************************************************************/
struct PluginInterface
{
    virtual ~PluginInterface() {}
    virtual void     initialize() = 0;
    virtual void     finalize() = 0;
    virtual QWidget* widget() = 0;
};



/** ***************************************************************************/
struct ExtensionInterface : public PluginInterface
{
    virtual ~ExtensionInterface() {}
    virtual void setupSession() {}
    virtual void teardownSession() {}
    virtual void setFuzzy(bool b) = 0;
    virtual void handleQuery(Query *q) = 0;
};
#define ALBERT_EXTENSION_IID "org.manuelschneid3r.albert.extension"
Q_DECLARE_INTERFACE(ExtensionInterface, ALBERT_EXTENSION_IID)



/** ***************************************************************************/
struct ExtensionProviderInterface : public PluginInterface
{
    virtual ~ExtensionProviderInterface() {}
};
#define ALBERT_EXTENSION_PROVIDER_IID "org.manuelschneid3r.albert.extensionprovider"
Q_DECLARE_INTERFACE(ExtensionProviderInterface, ALBERT_EXTENSION_PROVIDER_IID)



/** ***************************************************************************/
