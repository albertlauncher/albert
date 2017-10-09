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
#include <QStringList>
#include <QMap>
#include <QIcon>
#include "xdg_globals.h"

namespace XDG {

class EXPORT_XDG IconLookup
{
public:

    /**
     * @brief iconPath Does XDG icon lookup for the given icon name
     * @param iconName The icon name to lookup
     * @param themeName The theme to use
     * @return If an icon was found the path to the icon, else an empty string
     */
    static QString iconPath(QString iconName, QString themeName = QIcon::themeName());

    /**
     * @brief iconPath Does XDG icon lookup for the given icon names, stops on success
     * @param iconNames A list of icon names to lookup
     * @param themeName The theme to use
     * @return If one of the icons was found the path to the icon, else an empty string
     */
    static QString iconPath(std::initializer_list<QString> iconNames, QString themeName = QIcon::themeName());

private:

    IconLookup();
    static IconLookup *instance();

    QString themeIconPath(QString iconName, QString themeName = QIcon::themeName());
    QString doRecursiveIconLookup(const QString &iconName, const QString &theme, QStringList *checked);
    QString doIconLookup(const QString &iconName, const QString &themeFile);
    QString lookupThemeFile(const QString &themeName);

    QStringList iconDirs_;
    QMap<QString, QString> iconCache_;
};

}
