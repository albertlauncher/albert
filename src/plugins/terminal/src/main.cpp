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
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <set>
#include "main.h"
#include "xdgiconlookup.h"
#include "configwidget.h"
#include "query.h"
#include "standarditem.h"
#include "standardaction.h"
using std::pair;
using std::shared_ptr;
using std::vector;
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
    QString iconPath;
    bool dirtyFlag;
};



/** ***************************************************************************/
Terminal::Extension::Extension()
    : Core::Extension("org.albert.extension.terminal"),
      Core::QueryHandler(Core::Extension::id),
      d(new TerminalPrivate) {

    d->dirtyFlag = false;

    QString iconPath = XdgIconLookup::iconPath("terminal");
    d->iconPath = iconPath.isNull() ? ":terminal" : iconPath;

    connect(&d->watcher, &QFileSystemWatcher::directoryChanged, [this](){ d->dirtyFlag = true; });

    rebuildIndex();

}



/** ***************************************************************************/
Terminal::Extension::~Extension() {

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
        rebuildIndex();
}



/** ***************************************************************************/
void Terminal::Extension::handleQuery(Core::Query * query) {

    vector<pair<shared_ptr<Core::Item>,short>> results;

    // Drop the query
    QString actualQuery = query->searchTerm().mid(1);
    QString shell = QProcessEnvironment::systemEnvironment().value("SHELL");

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
        action->setText("Execute in the shell");
        action->setAction([shell, commandlineString](){
            QProcess::startDetached(QString("%1 -ic \"%2\"").arg(shell, commandlineString));
        });
        actions.push_back(std::move(action));

        action = std::make_shared<StandardAction>();
        action->setText("Execute in the terminal");
        action->setAction([shell, commandlineString](){
            QStringList cmdLineFields = terminalCommand.split(' ', QString::SkipEmptyParts);
            cmdLineFields.append(QString("%1 -ic '%2; exec %1'").arg(shell, commandlineString));
            QProcess::startDetached(cmdLineFields.takeFirst(), cmdLineFields);
        });
        actions.push_back(std::move(action));

        action = std::make_shared<StandardAction>();
        action->setText("Execute as root in terminal");
        action->setAction([shell, commandlineString](){
            QStringList cmdLineFields = terminalCommand.split(' ', QString::SkipEmptyParts);
            cmdLineFields.append(QString("%1 -ic 'sudo %2; exec %1'").arg(shell, commandlineString));
            QProcess::startDetached(cmdLineFields.takeFirst(), cmdLineFields);
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
void Terminal::Extension::rebuildIndex() {

    std::set<QString> index;

    // Index the executables in the path
    QStringList paths = QString(::getenv("PATH")).split(':', QString::SkipEmptyParts);
    for (const QString &path : paths) {
        QDirIterator dirIt(path);
        while (dirIt.hasNext()) {
            QFileInfo file(dirIt.next());
            if ( file.isExecutable() )
                index.insert(file.fileName());
        }
    }

    // If env contains the shell index the aliases
    QProcess process;
    process.start(QString("%1 -ic \"alias\"").arg(QProcessEnvironment::systemEnvironment().value("SHELL")));
    if ( !process.waitForFinished(100) )
        return;
    QTextStream standardout(process.readAllStandardOutput());
    QRegularExpression regex("(?<=alias )\\w*");
    while (!standardout.atEnd()){
        QString line = standardout.readLine();
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch())
            index.insert(match.captured(0));
    }

    d->index = std::move(index);
}
