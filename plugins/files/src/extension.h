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
#include <QObject>
#include "plugininterfaces/extension_if.h"
#include "search/search.h"
#include "fileindex.h"

namespace Files{
class ConfigWidget;
/** ***************************************************************************/
class Extension final : public QObject, public ExtensionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "../src/metadata.json")
    Q_INTERFACES(ExtensionInterface)

public:
    Extension();
    ~Extension();

    // ExtensionInterface
    void teardownSession();
    void handleQuery(Query*) override;
    void setFuzzy(bool b = true) override;

    // GenericPluginInterface
    void initialize() override;
    void finalize() override;
    QWidget *widget() override;

private:
    QPointer<ConfigWidget> _widget;
    FileIndex _fileIndex;
    Search<uint> _fileSearch;

    /* constexpr */
    static constexpr const char* CFG_FUZZY            = "Files/fuzzy";
    static constexpr const bool  CFG_FUZZY_DEF        = false;
};
}
