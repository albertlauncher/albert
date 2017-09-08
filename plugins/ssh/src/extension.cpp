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

#include <QDebug>
#include <QFileSystemWatcher>
#include <QFile>
#include <QDir>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <stdexcept>
#include <set>
#include <unistd.h>
#include <pwd.h>
#include "extension.h"
#include "configwidget.h"
#include "core/query.h"
#include "util/standardaction.h"
#include "util/standarditem.h"
#include "util/shutil.h"
#include "xdg/iconlookup.h"
using std::shared_ptr;
using std::vector;
using namespace Core;

extern QString terminalCommand;

namespace {

const char* CFG_USE_KNOWN_HOSTS = "use_known_hosts";
const bool  DEF_USE_KNOWN_HOSTS = true;

// Function to extract the hosts of a ssh config file
std::set<QString> getSshHostsFromConfig(const QString& path) {
    std::set<QString> hosts;
    QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while ( !in.atEnd() ) {
            QStringList fields = in.readLine().split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
            if ( fields.size() > 1 && fields[0] == "Host")
                for ( int i = 1; i < fields.size(); ++i ){
                    if ( !(fields[i].contains('*') || fields[i].contains('?')) )
                        hosts.insert(fields[i]);
                }
        }
        file.close();
    }
    return hosts;
}

// Function to extract the hosts of a ssh known_hosts file
std::set<QString> getSshHostsFromKnownHosts(const QString& path) {
    std::set<QString> hosts;
    QFile file(path);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QRegularExpression re ("^[a-zA-Z0-9\\.]*(?=(,.*)*\\s)");
        QTextStream in(&file);
        while ( !in.atEnd() ) {
            QString line = in.readLine();
            QRegularExpressionMatch match = re.match(line);
            if ( match.hasMatch() )
                hosts.insert(match.captured(0));
        }
        file.close();
    }
    return hosts;
}

}



class Ssh::Private
{
public:
    QString icon;
    QPointer<ConfigWidget> widget;
    QFileSystemWatcher fileSystemWatcher;
    vector<shared_ptr<Core::StandardItem>> hosts;
    QString shell;
    bool useKnownHosts;
};


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Ssh::Extension::Extension()
    : Core::Extension("org.albert.extension.ssh"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    // passwd must not be freed
    passwd *pwd = getpwuid(geteuid());
    if (pwd == NULL)
        throw "Could not retrieve user shell";
    d->shell = pwd->pw_shell;

    // Load settings
    d->useKnownHosts = settings().value(CFG_USE_KNOWN_HOSTS, DEF_USE_KNOWN_HOSTS).toBool();

    // Find ssh
    if (QStandardPaths::findExecutable("ssh").isNull())
        throw QString("[%s] ssh not found.").arg(Core::Plugin::id());

    // Find an appropriate icon
    d->icon = XDG::IconLookup::iconPath({"ssh", "terminal"});
    if (d->icon.isEmpty())
        d->icon = ":ssh"; // Fallback

    rescan();
}



/** ***************************************************************************/
Ssh::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Ssh::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Checkboxes
        d->widget->ui.checkBox_knownhosts->setChecked(useKnownHosts());
        connect(d->widget->ui.checkBox_knownhosts, &QCheckBox::toggled,
                this, &Extension::setUseKnownHosts);

        connect(d->widget->ui.pushButton_rescan, &QPushButton::clicked,
                this, &Extension::rescan);

    }
    return d->widget;
}



/** ***************************************************************************/
void Ssh::Extension::handleQuery(Core::Query * query) {

    if ( query->searchTerm().isEmpty() )
        return;

    // This extension must run only triggered
    if ( query->trigger().isNull() )
        return;

    QStringList queryTerms = query->searchTerm().split(' ',QString::SkipEmptyParts);

    // Add all hosts if there are no arguments
    if ( queryTerms.size() == 1)
        for ( shared_ptr<Core::StandardItem>& host : d->hosts )
            query->addMatch(host);  // Explicitly: No move

    if ( queryTerms.size() != 2)
        return;

    // Add all hosts that the query is a prefix of
    for ( shared_ptr<Core::StandardItem>& host : d->hosts )
        if ( host->text().startsWith(queryTerms[1]) )
            // Explicitly: No move
            query->addMatch(host, static_cast<uint>(1.0*query->searchTerm().size()/host->text().size()* UINT_MAX));

    // Add the quick connect item
    std::shared_ptr<StandardItem> item  = std::make_shared<StandardItem>("");
    item->setText(queryTerms[1]);
    item->setSubtext(QString("Quick connect to '%1' using ssh").arg(queryTerms[1]));
    item->setCompletionString(QString("ssh %1").arg(queryTerms[1]));
    item->setIconPath(d->icon);

    shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
    action->setText(QString("Connect to '%1' using ssh").arg(queryTerms[1]));
    action->setAction([queryTerms, this](){
        QStringList tokens;
        tokens << Core::ShUtil::split(terminalCommand)
               << d->shell << "-c"
               << QString(" ssh %1 || read -rsp $'\nPress enter to close the terminal.\n'")
                  .arg(queryTerms[1]);
        QProcess::startDetached(tokens.takeFirst(), tokens);
    });
    item->setActions({action});

    query->addMatch(std::move(item));
}



/** ***************************************************************************/
void Ssh::Extension::rescan() {

    // Build a new index
    vector<shared_ptr<StandardItem>> sshHosts;

    // Get the hosts in config
    std::set<QString> hosts;
    for ( const QString& path : {QString("/etc/ssh/config"), QDir::home().filePath(".ssh/config")} )
        if ( QFile::exists(path) )
            for ( const QString& host : getSshHostsFromConfig(path) )
                hosts.insert(host);

    // Get the hosts in known_hosts
    if ( d->useKnownHosts ) {
        const QString& path = QDir::home().filePath(".ssh/known_hosts");
        if ( QFile::exists(path) )
                for ( const QString& host : getSshHostsFromKnownHosts(path) )
                    hosts.insert(host);
    }


    for ( const QString& host : hosts ){

        // Create item
        std::shared_ptr<StandardItem> si  = std::make_shared<StandardItem>(host);
        si->setText(host);
        si->setSubtext(QString("Connect to '%1' using ssh").arg(host));
        si->setCompletionString(QString("ssh %1").arg(host));
        si->setIconPath(d->icon);

        shared_ptr<StandardAction> sa = std::make_shared<StandardAction>();
        sa->setText(QString("Connect to '%1' using ssh").arg(host));
        sa->setAction([host, this](){
            QStringList tokens;
            tokens << Core::ShUtil::split(terminalCommand)
                   << d->shell << "-c"
                   << QString(" ssh %1 || read -rsp $'\nPress enter to close the terminal.\n'").arg(host);
            QProcess::startDetached(tokens.takeFirst(), tokens);
        });
        si->setActions({sa});

        sshHosts.push_back(std::move(si));
    }

    d->hosts = std::move(sshHosts);
}



/** ***************************************************************************/
bool Ssh::Extension::useKnownHosts() {
    return d->useKnownHosts;
}



/** ***************************************************************************/
void Ssh::Extension::setUseKnownHosts(bool b) {
    settings().setValue(CFG_USE_KNOWN_HOSTS, b);
    d->useKnownHosts = b;
    rescan();
}
