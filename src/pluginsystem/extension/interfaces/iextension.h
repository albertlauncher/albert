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
#include <QWidget>
class IQuery;
class IExtensionManager;

struct IPlugin
{
    virtual ~IPlugin() {}
    virtual QWidget* widget() = 0;
};

struct IExtension : public IPlugin
{
    virtual ~IExtension() {}
    virtual void initialize(IExtensionManager *) {}
    virtual void finalize() {}
    virtual void setupSession() {}
    virtual void teardownSession() {}
    virtual void setFuzzy(bool b) = 0;
    virtual void handleQuery(IQuery *q) = 0;
};
#define ALBERT_EXTENSION_IID "org.manuelschneid3r.albert.extension"
Q_DECLARE_INTERFACE(IExtension, ALBERT_EXTENSION_IID)
