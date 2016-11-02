// albert - a simple application launcher for linux
// Copyright (C) 2016 Martin Buergmann
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
#include "extension.h"
#include "configwidget.h"
#include "abstractitem.h"
#include "query.h"
#include "albertapp.h"
#include "indexer.h"
#include <QStandardPaths>
#include <QSettings>
#include <QThreadPool>
#include <QFile>

#define CFG_GROUP               "firefoxbookmarks"

#define CFG_PROFILE             "profile"
#define DEF_PROFILE             false

#define MOZ_CFG_PROFILE_PATH    "Path"

/** ***************************************************************************/
const QVariant FirefoxBookmarks::Extension::nullVariant = QVariant::fromValue((QObject * const)nullptr);
//const char * const name_ = "Firefox Bookmarks";


/** ***************************************************************************/
FirefoxBookmarks::Extension::Extension() : AbstractExtension("org.albert.extension.firefoxbookmarks") {
    enabled_ = false;

    QString base = QStandardPaths::locate(QStandardPaths::HomeLocation, ".mozilla", QStandardPaths::LocateDirectory);
    if (base.isEmpty()) { // Try a windowsy approach
        base = QStandardPaths::locate(QStandardPaths::DataLocation, "Mozilla", QStandardPaths::LocateDirectory);
    }

    if (base.isEmpty()) {
        qWarning("[%s] Did not find mozilla base path, disabling", name_);
        return;      // We havn't found an applicable mozilla-location, probably firefox is not installed
    }

    QSettings *albertSettings = qApp->settings();
    albertSettings->beginGroup(CFG_GROUP);

    bool found = false;
    QVariant profilepath = albertSettings->value(CFG_PROFILE, DEF_PROFILE);
    if (!profilepath.toBool()) {
        QString profilesINIpath = base + "/firefox/profiles.ini";
        QFile profilesINIfile(profilesINIpath);

        if (!profilesINIfile.exists()) {
            qWarning("profiles.ini does not exist! This is weird!");
        }

        QSettings profilesINI(profilesINIpath, QSettings::IniFormat);

        profilesINI.beginGroup("Profile0");
        profilepath = profilesINI.value(MOZ_CFG_PROFILE_PATH, nullVariant);

        if (!profilepath.isNull()) {
            found = true; // We did not find a profile... wth?
            albertSettings->setValue(CFG_PROFILE, profilepath);
        }
    } else
        found = true;

    albertSettings->endGroup();
    if (!found) {
        qWarning("Could not determine the firefox profile path!");
        return;
    }

    QString sqliteName = "%1/firefox/%2/places.sqlite";
    sqliteName = sqliteName.arg(base).arg(profilepath.toString());

    qDebug() << sqliteName;

    base_ = QSqlDatabase::addDatabase("QSQLITE", "sqlite");
    base_.setDatabaseName(sqliteName);
    reloadConfig(sqliteName);

    placesWatcher_.addPath(sqliteName);
    connect(&placesWatcher_, SIGNAL(fileChanged(QString)), this, SLOT(reloadConfig(QString)));

    enabled_ = true;
}



/** ***************************************************************************/
FirefoxBookmarks::Extension::~Extension() {
    // Do sth.
}



/** ***************************************************************************/
QWidget *FirefoxBookmarks::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);
    }
    return widget_;
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::setupSession() {

}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::teardownSession() {

}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::handleQuery(Query query) {
    // Search for matches. Lock memory against indexer
    indexAccess_.lock();
    vector<shared_ptr<IIndexable>> indexables = offlineIndex_.search(query.searchTerm().toLower());
    indexAccess_.unlock();

    // Add results to query.
    for (const shared_ptr<IIndexable> &obj : indexables)
        // TODO `Search` has to determine the relevance. Set to 0 for now
        query.addMatch(std::static_pointer_cast<StandardIndexItem>(obj), 0);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::reloadConfig(QString) {
    Indexer *idxer = new Indexer(base_, this);
    idxer->setAutoDelete(true);

    QThreadPool::globalInstance()->start(idxer);
}

