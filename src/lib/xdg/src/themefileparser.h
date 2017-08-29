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
#include <QSettings>

namespace XDG {

class ThemeFileParser
{
public:

    ThemeFileParser(const QString &iniFile);

    QString path();
    QString name();
    QString comment();
    QStringList inherits();
    QStringList directories();
    bool hidden();
    int size(const QString& directory);
    QString context(const QString& directory);
    QString type(const QString& directory);
    int maxSize(const QString& directory);
    int minSize(const QString& directory);
    int threshold(const QString& directory);

private:

    QSettings iniFile_;

};

}
