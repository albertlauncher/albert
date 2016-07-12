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

#include <QDebug>
#include <QDirIterator>
#include <QProcess>
#include <QIcon>
#include <QStringList>
#include <QFileInfo>
#include <algorithm>
#include "extension.h"
#include "xdgiconlookup.h"
#include "configwidget.h"
#include "objects.hpp"
#include "query.h"

/** ***************************************************************************/
Terminal::Extension::Extension() : IExtension("org.albert.extension.terminal") {

    dirtyFlag_ = false;

    QString iconPath = XdgIconLookup::instance()->themeIconPath("terminal", QIcon::themeName());
    iconPath_ = iconPath.isNull() ? ":calc" : iconPath;

    connect(&watcher_, &QFileSystemWatcher::directoryChanged, [this](){ dirtyFlag_ = true; });

    rebuildIndex();

}



/** ***************************************************************************/
QWidget *Terminal::Extension::widget(QWidget *parent) {
    if (widget_.isNull())
        widget_ = new ConfigWidget(parent);
    return widget_;
}



/** ***************************************************************************/
void Terminal::Extension::teardownSession() {
    if ( dirtyFlag_ )
        // Build rebuild the chache
        rebuildIndex();
}



/** ***************************************************************************/
void Terminal::Extension::handleQuery(shared_ptr<Query> query) {

    QStringList arguments  = query->searchTerm().split(' ', QString::SkipEmptyParts);

    if (arguments.size() < 2)
        return;

    // Extract data from input string: [0] trigger [1] program. The rest: args
    arguments.takeFirst();
    QString potentialProgram = arguments.takeFirst();
    QString argumentsString = arguments.join(' ');

    // Search first match
    std::set<QString>::iterator it = std::lower_bound(index_.begin(), index_.end(), potentialProgram);

    // Iterate over matches
    QString program;
    while (it != index_.end() && it->startsWith(potentialProgram)){
        program = *it;
        std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>();
        item->setText(QString("%1 %2").arg(program, argumentsString));
        item->setSubtext(QString("Run '%1'").arg(item->text()));
        item->setIcon(iconPath_);
        item->setAction([program, arguments](){
            QProcess::startDetached(program, arguments);
        });
        query->addMatch(item, 0);
        ++it;
    }
}



/** ***************************************************************************/
void Terminal::Extension::rebuildIndex() {
    index_.clear();
    QStringList paths = QString(::getenv("PATH")).split(':', QString::SkipEmptyParts);
    for (const QString &path : paths) {
        QDirIterator dirIt(path);
        while (dirIt.hasNext()) {
            QFileInfo file(dirIt.next());
            if ( file.isExecutable() )
                index_.insert(file.fileName());
        }
    }
}
