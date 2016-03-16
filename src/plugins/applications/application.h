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
#include <QString>
#include <QIcon>
#include <vector>
using std::vector;
#include "search/iindexable.h"
#include "abstractobjects.hpp"

namespace Applications{

class DesktopAction;

class Application final : public AlbertItem, public IIndexable
{
    friend class Extension;
    friend class Indexer;

public:
    Application() = delete;
    Application(const Application &) = delete;
    Application(const QString &path, short usage = 0)
        : _path(path), _usage(usage) {}

    QString text() const override;
    QString subtext() const override;
    QIcon icon() const override;
    uint16_t usageCount() const override {return _usage;}
    void activate() override;
    bool hasChildren() const override;
    vector<shared_ptr<AlbertItem>> children() override;
    vector<QString> aliases() const override;

    bool readDesktopEntry();
    const QString& path() const {return _path;}
    void incUsage() {++_usage;}

    static QString terminal;

private:
    static QString escapeString(const QString &unescaped);
    static QString quoteString(const QString &unquoted);
    static QStringList execValueEscape(const QString &value);
    static QIcon getIcon(const QString &iconStr);

    QString _path;
    QString _name;
    QString _altName;
    QIcon   _icon;
    QString _exec;
    bool    _term;
    mutable ushort _usage;
    vector<shared_ptr<AlbertItem>> _actions;
};
}
