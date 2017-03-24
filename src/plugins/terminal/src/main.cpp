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
#include <QFutureWatcher>
#include <QDirIterator>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QtConcurrent>
#include <algorithm>
#include <set>
#include <pwd.h>
#include <unistd.h>
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

namespace  {

std::set<QString> scanCommands() {

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

    // passwd must not be freed
    passwd *pwd = getpwuid(geteuid());
    if (pwd == NULL)
        return index;

    // If env contains the shell index the aliases, aliases are sourced in interactive mode only
    QProcess process;
    process.start(QString("%1 -ic \"alias\"").arg(pwd->pw_shell));
    if ( !process.waitForFinished(2000) ||
         process.exitStatus() != QProcess::NormalExit ||
         process.exitCode() != 0 )
        return index;

    QTextStream standardout(process.readAllStandardOutput());
    QRegularExpression regex("(?:alias\\s)?(.+?)=");
    while (!standardout.atEnd()){
        QString line = standardout.readLine();
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch())
            index.insert(match.captured(1));
    }

    return index;
}

}

/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Terminal::Internal
{
public:
    QFutureWatcher<std::set<QString>> futureWatcher;
    QPointer<ConfigWidget> widget;
    QFileSystemWatcher fileSystemWatcher;
    std::set<QString> index;
    QString iconPath;
    bool dirtyFlag;

};



/** ***************************************************************************/
void Terminal::Extension::startIndexing() {

    // Never run concurrent
    if ( d->futureWatcher.future().isRunning() )
        return;

    // Run finishIndexing when the indexing thread finished
    d->futureWatcher.disconnect();
    connect(&d->futureWatcher, &QFutureWatcher<std::set<QString>>::finished,
            this, &Extension::finishIndexing);

    // Run the indexer thread
    d->futureWatcher.setFuture(QtConcurrent::run(scanCommands));

    // Notification
    qDebug() << "Start indexing programs and aliases.";
    emit statusInfo("Indexing programs and aliases ...");
}



/** ***************************************************************************/
void Terminal::Extension::finishIndexing() {

    // Get the thread results
    d->index = d->futureWatcher.future().result();

    // Update filesystem watchers paths
    QStringList paths = QString(::getenv("PATH")).split(':', QString::SkipEmptyParts);
    if ( !d->fileSystemWatcher.directories().isEmpty() )
        d->fileSystemWatcher.removePaths(d->fileSystemWatcher.directories());
    d->fileSystemWatcher.addPaths(paths);

    // Notification
    qDebug() << qPrintable(QString("Indexed %1 programs and aliases.").arg(d->index.size()));
    emit statusInfo(QString("%1 programs and aliases indexed.").arg(d->index.size()));
}



/** ***************************************************************************/
Terminal::Extension::Extension()
    : Core::Extension("org.albert.extension.terminal"),
      Core::QueryHandler(Core::Extension::id),
      d(new Internal) {

    d->dirtyFlag = false;

    QString iconPath = XdgIconLookup::iconPath("terminal");
    d->iconPath = iconPath.isNull() ? ":terminal" : iconPath;

    connect(&d->fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &Extension::startIndexing);

    startIndexing();

}



/** ***************************************************************************/
Terminal::Extension::~Extension() {
    d->futureWatcher.waitForFinished();
}



/** ***************************************************************************/
QWidget *Terminal::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {

        d->widget = new ConfigWidget(parent);

        // Status bar
        ( d->futureWatcher.isRunning() )
            ? d->widget->ui.label_statusbar->setText("Indexing programs and aliases ...")
            : d->widget->ui.label_statusbar->setText(QString("%1 programs and aliases indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, d->widget->ui.label_statusbar, &QLabel::setText);
    }

    return d->widget;
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
        item->setCompletionString(QString(">%1").arg(commandlineString));
        item->setIconPath(d->iconPath);
        item->setActions(std::move(actions));

        results.emplace_back(item, 0);
        ++it;
    }

    // Add results to query
    query->addMatches(results.begin(), results.end());
}

