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

#include <QDesktopServices>
#include <QUrl>
#include <QIcon>
#include "openfileaction.h"
#include "file.h"
#include "albertapp.h"

unsigned short Files::OpenFileAction::usageCounter = 0;

/** ***************************************************************************/
QVariant Files::OpenFileAction::data(int role) const  {
    switch (role) {
    case Qt::DisplayRole:
        return "Open file in default application";
    case Qt::ToolTipRole:
        return _file->path;
    case Qt::DecorationRole:
        if (QIcon::hasThemeIcon(_file->mimetype.iconName()))
            return QIcon::fromTheme(_file->mimetype.iconName());
        else if(QIcon::hasThemeIcon(_file->mimetype.genericIconName()))
            return QIcon::fromTheme(_file->mimetype.genericIconName());
        else
            return QIcon::fromTheme("unknown");
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
void Files::OpenFileAction::activate() {
    QDesktopServices::openUrl(QUrl("file://" + _file->path));
    qApp->hideWidget();
}



/** ***************************************************************************/
unsigned short Files::OpenFileAction::score() const {
    return usageCounter;
}

