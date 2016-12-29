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
#include <QPluginLoader>
#include "extensionloader.h"
#include "core_globals.h"

namespace Core {

class Extension;

class EXPORT_CORE NativeExtensionLoader final : public Core::ExtensionLoader
{
public:

    NativeExtensionLoader(QString path) : loader_(path) { }
    ~NativeExtensionLoader() { }

    bool load() override;
    bool unload() override;
    QString lastError() const override;
    Extension *instance() override;
    QString path() const override;
    QString type() const override;
    // Metadata
    QString id() const override;
    QString name() const override;
    QString version() const override;
    QString author() const override;
    QStringList dependencies() const override;

private:

    QPluginLoader loader_;

};

}
