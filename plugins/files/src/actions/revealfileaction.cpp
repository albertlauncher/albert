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

#include <QApplication>
#include <QStyle>
#include <QDesktopServices>
#include <QUrl>
#include "revealfileaction.h"
#include "albertapp.h"

unsigned int Files::RevealFileAction::usageCounter = 0;

/** ***************************************************************************/
QString Files::RevealFileAction::name(const Query *q) const {
    Q_UNUSED(q);
    return "Reveal file in default filebrowser";
}



/** ***************************************************************************/
QString Files::RevealFileAction::description(const Query *q) const {
    Q_UNUSED(q);
    return _file->absolutePath();
}



/** ***************************************************************************/
QIcon Files::RevealFileAction::icon() const {
    return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
}



/** ***************************************************************************/
void Files::RevealFileAction::activate(const Query *q) {
    Q_UNUSED(q);
    QDesktopServices::openUrl(QUrl("file://" + _file->path() + "/"));
    qApp->hideWidget();
}



/** ***************************************************************************/
uint Files::RevealFileAction::usage() const {
    return usageCounter;
}
