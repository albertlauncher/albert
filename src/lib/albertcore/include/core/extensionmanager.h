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
#include <set>
#include <vector>
#include <memory>
#include "extension.h"
#include "core_globals.h"

namespace Core {

class Extension;
class PluginSpec;
class ExtensionManagerPrivate;

class EXPORT_CORE ExtensionManager final : public QObject
{
    Q_OBJECT

public:

    ExtensionManager(std::vector<std::unique_ptr<PluginSpec>> && pluginSpecs, QObject *parent = 0);
    ~ExtensionManager();

    const std::vector<std::unique_ptr<PluginSpec>> & extensionSpecs() const;

    void reloadExtensions();

    void enableExtension(const std::unique_ptr<PluginSpec> &);
    void disableExtension(const std::unique_ptr<PluginSpec> &);
    bool extensionIsEnabled(const std::unique_ptr<PluginSpec> &);

    void registerObject(QObject *);
    void unregisterObject(QObject*);
    const std::set<QObject *> objects() const;
    template <typename T>
    std::set<T *> objectsByType() {
        std::set<T *> results;
        for (QObject * object : objects()) {
            T *result = dynamic_cast<T *>(object);
            if (result)
                results.insert(result);
        }
        return results;
    }

    static ExtensionManager *instance;

private:

    void loadExtension(const std::unique_ptr<PluginSpec> &);
    void unloadExtension(const std::unique_ptr<PluginSpec> &);

    std::unique_ptr<ExtensionManagerPrivate> d;

signals:

    void extensionLoaded(Extension*);
    void extensionAboutToUnload(Extension*);

};

}
