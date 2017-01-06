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
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QPointer>
#include <QProcess>
#include <QStringList>
#include <algorithm>
#include <set>
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



class Terminal::TerminalPrivate
{
public:
    QPointer<ConfigWidget> widget;
    QFileSystemWatcher watcher;
    std::set<QString> index;
    bool dirtyFlag;
    QString iconPath;

    void rebuildIndex();
};



/** ***************************************************************************/
Terminal::Extension::Extension()
    : Core::Extension("org.albert.extension.terminal"),
      Core::QueryHandler(Core::Extension::id),
      d(new TerminalPrivate) {

    d->dirtyFlag = false;

    QString iconPath = XdgIconLookup::instance()->themeIconPath("terminal");
    d->iconPath = iconPath.isNull() ? ":calc" : iconPath;

    connect(&d->watcher, &QFileSystemWatcher::directoryChanged, [this](){ d->dirtyFlag = true; });

    d->rebuildIndex();

}



/** ***************************************************************************/
Terminal::Extension::~Extension() {
    delete d;
}



/** ***************************************************************************/
QWidget *Terminal::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(parent);
    return d->widget;
}



/** ***************************************************************************/
void Terminal::Extension::teardownSession() {
    if ( d->dirtyFlag )
        // Build rebuild the chache
        d->rebuildIndex();
}



/** ***************************************************************************/
void Terminal::Extension::handleQuery(Core::Query * query) {

    vector<pair<shared_ptr<Core::Item>,short>> results;

    // Drop the query
    QString actualQuery = query->searchTerm().mid(1);

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = actualQuery.section(' ', 0, 0, QString::SectionSkipEmpty);
    QString argsString = actualQuery.section(' ', 1, -1, QString::SectionSkipEmpty);
    QStringList args = argsString.split(' ', QString::SkipEmptyParts);

    // Search first match
    std::set<QString>::iterator it = std::lower_bound(d->index.begin(), d->index.end(), potentialProgram);

    // Iterate over matches
    QString program;
     while (it != d->index.end() && it->startsWith(potentialProgram)){
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
        item->setIconPath(d->iconPath);
        item->setActions(std::move(actions));

        results.emplace_back(item, 0);
        ++it;
    }

    // Add results to query
    query->addMatches(results.begin(), results.end());
}



/** ***************************************************************************/
void Terminal::TerminalPrivate::rebuildIndex() {
    index.clear();
    QStringList paths = QString(::getenv("PATH")).split(':', QString::SkipEmptyParts);
    for (const QString &path : paths) {
        QDirIterator dirIt(path);
        while (dirIt.hasNext()) {
            QFileInfo file(dirIt.next());
            if ( file.isExecutable() )
                index.insert(file.fileName());
        }
    }
}
