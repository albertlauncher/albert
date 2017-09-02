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

#include <QDebug>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QPointer>
#include <QFuture>
#include <QFutureWatcher>
#include <QProcess>
#include <QStringList>
#include <QtConcurrent>
#include <set>
#include <pwd.h>
#include <unistd.h>
#include "extension.h"
#include "configwidget.h"
#include "core/query.h"
#include "util/shutil.h"
#include "util/standarditem.h"
#include "util/standardaction.h"
#include "xdg/iconlookup.h"
using namespace std;
using Core::Action;
using Core::StandardAction;
using Core::StandardItem;

extern QString terminalCommand;

/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Terminal::Private
{
public:
    QPointer<ConfigWidget> widget;
    QString iconPath;
    QString shell;
    QFileSystemWatcher watcher;
    set<QString> index;
    QFutureWatcher<set<QString>> futureWatcher;
};



/** ***************************************************************************/
Terminal::Extension::Extension()
    : Core::Extension("org.albert.extension.terminal"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    QString iconPath = XDG::IconLookup::iconPath("terminal");
    d->iconPath = iconPath.isNull() ? ":terminal" : iconPath;

    d->watcher.addPaths(QString(::getenv("PATH")).split(':', QString::SkipEmptyParts));
    connect(&d->watcher, &QFileSystemWatcher::directoryChanged,
            this, &Extension::rebuildIndex);

    // passwd must not be freed
    passwd *pwd = getpwuid(geteuid());
    if (pwd == NULL)
        throw "Could not retrieve user shell";
    d->shell = pwd->pw_shell;

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
void Terminal::Extension::handleQuery(Core::Query * query) {

    if ( query->searchTerm().isEmpty() )
        return;

    // This extension must run only triggered
    if ( query->trigger().isNull() )
        return;

    QString commandline = query->searchTerm().mid(1).trimmed();
    if (commandline.isEmpty())
        return;

    vector<pair<shared_ptr<Core::Item>,short>> results;

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = commandline.section(' ', 0, 0, QString::SectionSkipEmpty);
    QString argsString = commandline.section(' ', 1, -1, QString::SectionSkipEmpty);

    // Iterate over matches
    set<QString>::iterator it = lower_bound(d->index.begin(), d->index.end(), potentialProgram);

    // Get the
    QString commonPrefix;
    if ( it != d->index.end() )
        commonPrefix = *it;

    while (it != d->index.end() && it->startsWith(potentialProgram)){

        // Update common prefix
        auto mismatchindexes = std::mismatch(it->begin()+potentialProgram.size()-1, it->end(),
                                             commonPrefix.begin()+potentialProgram.size()-1);
        commonPrefix.resize(std::distance(it->begin(), mismatchindexes.first));

        QString commandlineString = QString("%1 %2").arg(*it, argsString);

        vector<shared_ptr<Action>> actions;

        actions.push_back(make_shared<StandardAction>("Execute in your  shell", [this, commandlineString](){
            QProcess::startDetached(d->shell, {"-ic", commandlineString});
        }));

        actions.push_back(make_shared<StandardAction>("Execute in the terminal", [=](){
            QStringList tokens = Core::ShUtil::split(terminalCommand);
            tokens << d->shell << "-ic" << QString("%1; exec %2").arg(commandlineString, d->shell);
            QProcess::startDetached(tokens.takeFirst(), tokens);
        }));

        // Build Item
        shared_ptr<StandardItem> item = make_shared<StandardItem>(*it);
        item->setText(commandlineString);
        item->setSubtext(QString("Run '%1' in your shell").arg(commandlineString));
        item->setIconPath(d->iconPath);
        item->setActions(move(actions));

        results.emplace_back(item, 0);
        ++it;
    }

    // Apply completion string to items
    QString completion = QString(">%1").arg(commonPrefix);
    for (pair<shared_ptr<Core::Item>,short> &match: results)
        std::static_pointer_cast<StandardItem>(match.first)->setCompletionString(completion);

    // Build Item
    vector<shared_ptr<Action>> actions;
    actions.push_back(make_shared<StandardAction>("Execute in the shell",
                                                       [this, commandline](){
        QProcess::startDetached(d->shell, {"-ic", commandline});
    }));
     actions.push_back(make_shared<StandardAction>("Execute in the terminal", [=](){
        QStringList tokens = Core::ShUtil::split(terminalCommand);
        tokens << d->shell << "-ic" << QString("%1; exec %2").arg(commandline, d->shell);
        QProcess::startDetached(tokens.takeFirst(), tokens);
    }));

    shared_ptr<StandardItem> item = make_shared<StandardItem>();
    item->setText(commandline);
    item->setSubtext(QString("Try running '%1' in your shell").arg(commandline));
    item->setCompletionString(query->searchTerm());
    item->setIconPath(d->iconPath);
    item->setActions(move(actions));

    results.emplace_back(item, 0);

    // Add results to query
    query->addMatches(results.begin(), results.end());
}


/** ***************************************************************************/
void Terminal::Extension::rebuildIndex() {

    if ( d->futureWatcher.isRunning() )
        return;

    auto index = [](){
        qDebug() << "Building $PATH index.";
        set<QString> index;
        QStringList paths = QString(::getenv("PATH")).split(':', QString::SkipEmptyParts);
        for (const QString &path : paths) {
            QDirIterator dirIt(path);
            while (dirIt.hasNext()) {
                QFileInfo file(dirIt.next());
                if ( file.isExecutable() )
                    index.insert(file.fileName());
            }
        }
        qDebug() << "Building $PATH index done.";
        return index;
    };

    connect(&d->futureWatcher, &QFutureWatcher<set<QString>>::finished, this, [this](){
        d->index = d->futureWatcher.future().result();
        disconnect(&d->futureWatcher,0,0,0);
    });

    d->futureWatcher.setFuture(QtConcurrent::run(index));
}
