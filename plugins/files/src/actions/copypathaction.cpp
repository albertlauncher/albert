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

#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include "copypathaction.h"
#include "albertapp.h"

unsigned int Files::CopyPathAction::usageCounter = 0;

/** ***************************************************************************/
QString Files::CopyPathAction::name(const Query *q) const {
    Q_UNUSED(q);
    return "Copy path to clipboard";
}



/** ***************************************************************************/
QString Files::CopyPathAction::description(const Query *q) const {
    Q_UNUSED(q);
    return _file->absolutePath();
}



/** ***************************************************************************/
QIcon Files::CopyPathAction::icon() const {
    return QIcon::fromTheme("edit-copy");
}



/** ***************************************************************************/
void Files::CopyPathAction::activate(const Query *q) {
    Q_UNUSED(q);
    // Ownership of the data is transferred to the clipboard.
    QApplication::clipboard()->setText(_file->absolutePath());
    qApp->hideWidget();
}



/** ***************************************************************************/
uint Files::CopyPathAction::usage() const {
    return usageCounter;
}
