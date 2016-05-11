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
#include <QStringList>
#include <QMap>

class XdgIconLookup
{
public:

    QString themeIconPath(QString iconName, QString themeName);
    static XdgIconLookup *instance();

private:

    XdgIconLookup();

    QString doRecursiveIconLookup(const QString &iconName, const QString &theme, QStringList *checked);
    QString doIconLookup(const QString &iconName, const QString &themeFile);
    QString lookupThemeFile(const QString &themeName);

    QStringList iconDirs_;
    QMap<QString, QString> iconCache_;

    static QStringList icon_extensions;
    static XdgIconLookup *instance_;

};
