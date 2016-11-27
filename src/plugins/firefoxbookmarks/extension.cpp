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
#include <QComboBox>
#include <QCheckBox>

#define CFG_GROUP               "firefoxbookmarks"

#define CFG_PROFILE             "profile"
#define DEF_PROFILE             false
#define CFG_FUZZY               "fuzzy"
#define DEF_FUZZY               true
#define CFG_WITH_FIREFOX        "openWithFirefox"
#define DEF_WITH_FIREFOX        true

#define MOZ_CFG_PROFILE_PATH    "Path"

/** ***************************************************************************/
const QVariant FirefoxBookmarks::Extension::nullVariant = QVariant::fromValue((QObject * const)nullptr);
//const char * const name_ = "Firefox Bookmarks";


/** ***************************************************************************/
FirefoxBookmarks::Extension::Extension() : AbstractExtension("org.albert.extension.firefoxbookmarks"), indexing_(false), coolingdown_(false), tmpBase_(nullptr) {
    enabled_ = false;

    // Locate mozilla directory
    QString base = QStandardPaths::locate(QStandardPaths::HomeLocation, ".mozilla", QStandardPaths::LocateDirectory);
    if (base.isEmpty()) { // Try a windowsy approach
        base = QStandardPaths::locate(QStandardPaths::DataLocation, "Mozilla", QStandardPaths::LocateDirectory);
    }

    if (base.isEmpty()) {
        qWarning("[%s] Did not find mozilla base path, disabling", name_);
        return;      // We havn't found an applicable mozilla-location, probably firefox is not installed
    }

    // Check for firefox executable
    firefoxExeFound_ = !QStandardPaths::findExecutable("firefox").isEmpty();

    // Setup some path variables
    profileBasePath_ = base + "/firefox";
    profilesIniPath_ = profileBasePath_ + "/profiles.ini";
    QFile profilesINIfile(profilesIniPath_);

    if (!profilesINIfile.exists()) {
        qWarning("profiles.ini does not exist! This is weird!");
    } else {
        profilesWatcher_.addPath(profilesIniPath_);
        connect(&profilesWatcher_, SIGNAL(fileChanged(QString)), this, SLOT(scanProfiles(QString)));
    }

    QSettings *albertSettings = qApp->settings();
    albertSettings->beginGroup(CFG_GROUP);

    // Which profiles do we have?
    scanProfiles(profilesIniPath_);

    if (profiles_.length() == 0) {
        qWarning("No Firefox profiles found! Disabling . . .");
        return;
    }

    // Which profile did we use last time?
    QVariant profilepath = albertSettings->value(CFG_PROFILE, false);
    if (!profilepath.toBool()) {
        // None, so just use the first you can grab
        albertSettings->setValue(CFG_PROFILE, profiles_.at(0));
        currentProfile_ = profiles_.at(0);
    } else {
        currentProfile_ = profilepath.toString();
    }

    offlineIndex_.setFuzzy(albertSettings->value(CFG_FUZZY, DEF_FUZZY).toBool());

    openWithFirefox_ = albertSettings->value(CFG_WITH_FIREFOX, DEF_WITH_FIREFOX).toBool();

    albertSettings->endGroup();


    base_ = QSqlDatabase::addDatabase("QSQLITE", "sqlite");
    changeProfile(currentProfile_);

    cooldown_.setInterval(60000);
    cooldown_.setSingleShot(true);
    connect(&cooldown_, SIGNAL(timeout()), this, SLOT(cooldownFinished()));

    connect(&placesWatcher_, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

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

        QComboBox *cmb = widget_->ui.comboBox;
        cmb->addItems(profiles_);
        cmb->setCurrentIndex(profiles_.indexOf(currentProfile_));
        connect(cmb, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeProfile(QString)));

        QCheckBox *ckb = widget_->ui.fuzzy;
        ckb->setChecked(offlineIndex_.fuzzy());
        connect(ckb, SIGNAL(clicked(bool)), this, SLOT(changeFuzzyness(bool)));

        ckb = widget_->ui.openWithFirefox;
        ckb->setChecked(openWithFirefox_);
        connect(ckb, SIGNAL(clicked(bool)), this, SLOT(changeOpenPolicy(bool)));
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
    for (const shared_ptr<IIndexable> &obj : indexables) {
        // TODO `Search` has to determine the relevance. Set to 0 for now
        query.addMatch(std::static_pointer_cast<StandardIndexItem>(obj), 0);
    }
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::cooldownFinished() {
    coolingdown_ = false;
    if (rescan_)
        reloadConfig();
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::fileChanged(QString) {
    if (indexing_ || coolingdown_)
        rescan_ = true;
    else
        reloadConfig();
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::reloadConfig() {
    // Copy here the base to a tmpfile and use it as db
    // disconnect the watcher for the time it's indexing
    // delete the tmpfile
    // start a cooldown timer
    // reconnect the watcher
    QTemporaryFile tmpFile;
    tmpFile.open();
    tmpFile.setAutoRemove(false);

    QFile dbFile;
    dbFile.open(QFile::ReadOnly);

    const qint64 chunk_size = 4*1024*1024;// chunksize 4MiB
    char *buf = new char[chunk_size];
    qint64 read;
    while ((read = dbFile.read(buf, chunk_size)) > chunk_size) {
        tmpFile.write(buf, read);
    }

    tmpFile.close();

    base_.setDatabaseName(tmpFile.fileName());
    Indexer *idxer = new Indexer(base_, this);
    idxer->setAutoDelete(true);

    QThreadPool::globalInstance()->start(idxer);
    indexing_ = true;
    coolingdown_ = false;
    rescan_ = false;
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::scanProfiles(QString profilesIni) {

    profiles_.clear();

    QSettings profiles(profilesIni, QSettings::IniFormat);
    QStringList allkeys = profiles.allKeys();

    for (QString &key : allkeys) {
        if (key.endsWith("Path")) {
            profiles_.append(profiles.value(key).toString());
        }
    }

}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::scanBookmarksFinished() {
    indexing_ = false;

    coolingdown_ = true;
    cooldown_.start();
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeProfile(QString profile) {
    QString sqliteName = profileBasePath_ + "/%1/places.sqlite";
    sqliteName = sqliteName.arg(profile);

    base_.setDatabaseName(sqliteName);
    reloadConfig();

    placesWatcher_.removePaths(placesWatcher_.files());
    placesWatcher_.addPath(sqliteName);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeFuzzyness(bool fuzzy) {
    offlineIndex_.setFuzzy(fuzzy);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeOpenPolicy(bool withFirefox) {
    openWithFirefox_ = withFirefox;
    reloadConfig();
}

