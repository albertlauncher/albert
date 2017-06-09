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
#include <QPointer>
#include <QProcess>
#include <QStringList>
#include <pwd.h>
#include <unistd.h>
#include "main.h"
#include "xdgiconlookup.h"
#include "configwidget.h"
#include "query.h"
#include "standarditem.h"
#include "standardaction.h"
#include "shlex.h"
using std::shared_ptr;
using Core::Action;
using Core::StandardAction;
using Core::StandardItem;

extern QString terminalCommand;

/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Terminal::Internal
{
public:
    QPointer<ConfigWidget> widget;
    QString iconPath;
};



/** ***************************************************************************/
Terminal::Extension::Extension()
    : Core::Extension("org.albert.extension.terminal"),
      Core::QueryHandler(Core::Extension::id),
      d(new Internal) {

    QString iconPath = XdgIconLookup::iconPath("terminal");
    d->iconPath = iconPath.isNull() ? ":terminal" : iconPath;
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

    // This extension must run only triggered
    if ( !query->isTriggered() )
        return;

    // passwd must not be freed
    passwd *pwd = getpwuid(geteuid());
    if (pwd == NULL){
        qWarning() << "Could not retrieve user shell";
        return;
    }
    QString commandline = query->searchTerm().mid(1).trimmed();
    if (commandline.isEmpty())
        return;

    QString shell(pwd->pw_shell);

    // Build Item
    std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>();
    item->setText(commandline);
    item->setSubtext(QString("Run '%1' in terminal").arg(commandline));
    item->setCompletionString(query->searchTerm());
    item->setIconPath(d->iconPath);

    std::vector<shared_ptr<Action>> actions;
    actions.push_back(std::make_shared<StandardAction>("Execute in the shell",
                                                       [shell, commandline](){
        QProcess::startDetached(shell, {"-c", commandline});
    }));
     actions.push_back(std::make_shared<StandardAction>("Execute in the terminal", [=](){
        QStringList tokens = Util::ShellLexer::split(terminalCommand);
        tokens << shell << "-ic" << QString("%1; exec %2").arg(commandline, shell);
        QProcess::startDetached(tokens.takeFirst(), tokens);
    }));

    item->setActions(std::move(actions));

    // Add results to query
    query->addMatch(item, 0);
}

