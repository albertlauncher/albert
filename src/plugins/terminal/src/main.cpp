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
#include <QStringList>
#include <QFileInfo>
#include <algorithm>
#include "main.h"
#include "xdgiconlookup.h"
#include "configwidget.h"
#include "query.h"
#include "standarditem.h"
#include "standardaction.h"
using Core::Action;
using Core::StandardAction;
using Core::StandardItem;

extern QString terminalCommand;

/** ***************************************************************************/
Terminal::Extension::Extension() : Core::Extension("org.albert.extension.terminal") {

    dirtyFlag_ = false;

    QString iconPath = XdgIconLookup::instance()->themeIconPath("terminal");
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
void Terminal::Extension::handleQuery(Core::Query * query) {

    // Drop the query
    QString actualQuery = query->searchTerm().mid(1);

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = actualQuery.section(' ', 0, 0, QString::SectionSkipEmpty);
    QString argsString = actualQuery.section(' ', 1, -1, QString::SectionSkipEmpty);
    QStringList args = argsString.split(' ', QString::SkipEmptyParts);

    // Search first match
    std::set<QString>::iterator it = std::lower_bound(index_.begin(), index_.end(), potentialProgram);

    // Iterate over matches
    QString program;
     while (it != index_.end() && it->startsWith(potentialProgram)){
        program = *it;
        QString commandlineString = QString("%1 %2").arg(program, argsString);

        std::vector<shared_ptr<Action>> actions;
        shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
        action->setText("Execute in background");
        action->setAction([program, args](){
            QProcess::startDetached(program, args);
        });
        actions.push_back(std::move(action));

        QStringList cmddline = terminalCommand.split(' ', QString::SkipEmptyParts);
        cmddline.append(program);
        cmddline.append(args);
        action = std::make_shared<StandardAction>();
        action->setText("Execute in terminal");
        action->setAction([cmddline](){
            QStringList args = cmddline;
            QString program = args.takeFirst();
            QProcess::startDetached(program, args);
        });
        actions.push_back(std::move(action));

        cmddline = terminalCommand.split(' ', QString::SkipEmptyParts);
        cmddline.append("sudo");
        cmddline.append(program);
        cmddline.append(args);
        action = std::make_shared<StandardAction>();
        action->setText("Execute as root in terminal");
        action->setAction([cmddline](){
            QStringList args = cmddline;
            QString program = args.takeFirst();
            QProcess::startDetached(program, args);
        });
        actions.push_back(std::move(action));


        std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>(program);
        item->setText(commandlineString);
        item->setSubtext(QString("Run '%1'").arg(commandlineString));
        item->setIconPath(iconPath_);
        item->setActions(std::move(actions));

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
