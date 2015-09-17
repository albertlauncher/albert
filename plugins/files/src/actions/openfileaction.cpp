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
#include "openfileaction.h"
#include "albertapp.h"

unsigned int Files::OpenFileAction::usageCounter = 0;

/** ***************************************************************************/
QString Files::OpenFileAction::name(const Query *q) const {
    Q_UNUSED(q);
    return "Open file in default application";
}



/** ***************************************************************************/
QString Files::OpenFileAction::description(const Query *q) const {
    Q_UNUSED(q);
    return _file->absolutePath();
}



/** ***************************************************************************/
QIcon Files::OpenFileAction::icon() const {
    return parent()->icon();
}



/** ***************************************************************************/
void Files::OpenFileAction::activate(const Query *q) {
    Q_UNUSED(q);
    QDesktopServices::openUrl(QUrl("file://" + _file->absolutePath()));
    qApp->hideWidget();
}



/** ***************************************************************************/
uint Files::OpenFileAction::usage() const {
    return usageCounter;
}

