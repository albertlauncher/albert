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
#include <QObject>
#include <QString>
#include <QStringList>
#include <set>
#include <vector>
#include <memory>
#include "extension.h"
#include "core_globals.h"
using std::set;
using std::vector;
using std::unique_ptr;

namespace Core {

class Extension;
class ExtensionSpec;
class ExtensionManagerPrivate;

class EXPORT_CORE ExtensionManager final : public QObject
{
    Q_OBJECT

public:

    ExtensionManager();
    ~ExtensionManager();

    void reloadExtensions();
    const vector<unique_ptr<ExtensionSpec>> & extensionSpecs() const;
    void enableExtension(const unique_ptr<ExtensionSpec> &);
    void disableExtension(const unique_ptr<ExtensionSpec> &);
    bool extensionIsEnabled(const unique_ptr<ExtensionSpec> &);

    void registerObject(QObject *);
    void unregisterObject(QObject*);
    const set<QObject *> objects() const;
    template <typename T>
    set<T *> objectsByType() {
        set<T *> results;
        for (QObject * object : objects()) {
            T *result = dynamic_cast<T *>(object);
            if (result)
                results.insert(result);
        }
        return results;
    }

    static ExtensionManager *instance;

private:

    void loadExtension(const unique_ptr<ExtensionSpec> &);
    void unloadExtension(const unique_ptr<ExtensionSpec> &);

    unique_ptr<ExtensionManagerPrivate> d;

signals:

    void extensionLoaded(Extension*);
    void extensionAboutToUnload(Extension*);

};

}
