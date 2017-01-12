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


#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QtConcurrent>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QThreadPool>
#include <QUrl>
#include <functional>
#include <map>
#include "main.h"
#include "configwidget.h"
#include "extension.h"
#include "item.h"
#include "offlineindex.h"
#include "standardaction.h"
#include "standardindexitem.h"
#include "query.h"
using std::shared_ptr;
using std::vector;
using namespace Core;

namespace {
const QString CFG_PROFILE = "profile";
const QString CFG_FUZZY   = "fuzzy";
const bool    DEF_FUZZY   = false;
const QString CFG_USE_FIREFOX   = "openWithFirefox";
const bool    DEF_USE_FIREFOX   = false;
}



class FirefoxBookmarks::FirefoxBookmarksPrivate
{
public:
    bool openWithFirefox;
    QPointer<ConfigWidget> widget;
    QString firefoxExecutable;
    QString profilesIniPath;
    QString currentProfileId;
    QFileSystemWatcher databaseWatcher;

    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;
    vector<shared_ptr<Core::StandardIndexItem>> index;
    Core::OfflineIndex offlineIndex;
};



/** ***************************************************************************/
FirefoxBookmarks::Extension::Extension()
    : Core::Extension("org.albert.extension.firefoxbookmarks"),
      Core::QueryHandler(Core::Extension::id),
      d(new FirefoxBookmarksPrivate){

    // Add a sqlite database connection for this extension, check requirements
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", Core::Extension::id);
    if ( !db.isValid() )
        throw QString("[%s] Firefox executable not found.").arg(Core::Extension::id);
    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
        throw QString("[%s] Firefox executable not found.").arg(Core::Extension::id);

    // Find firefox executable
    d->firefoxExecutable = QStandardPaths::findExecutable("firefox");
    if (d->firefoxExecutable.isEmpty())
        throw QString("[%s] Firefox executable not found.").arg(Core::Extension::id);

    // Locate profiles ini
    d->profilesIniPath = QStandardPaths::locate(QStandardPaths::HomeLocation,
                                                 ".mozilla/firefox/profiles.ini",
                                                 QStandardPaths::LocateFile);
    if (d->profilesIniPath.isEmpty()) // Try a windowsy approach
        d->profilesIniPath = QStandardPaths::locate(QStandardPaths::DataLocation,
                                                     "Mozilla/firefox/profiles.ini",
                                                     QStandardPaths::LocateFile);
    if (d->profilesIniPath.isEmpty())
        throw QString("[%1] Could not locate profiles.ini.").arg(Core::Extension::id);

    // Load the settings
    QSettings s(qApp->applicationName());
    s.beginGroup(Core::Extension::id);
    d->currentProfileId = s.value(CFG_PROFILE).toString();
    d->offlineIndex.setFuzzy(s.value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->openWithFirefox = s.value(CFG_USE_FIREFOX, DEF_USE_FIREFOX).toBool();

    // If the id does not exist find a proper default
    QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);
    if ( !profilesIni.contains(d->currentProfileId) ){

        d->currentProfileId = QString();

        QStringList ids = profilesIni.childGroups();
        if ( ids.isEmpty() )
            qWarning() << qPrintable(QString("[%1] No Firefox profiles found.").arg(Core::Extension::id));
        else {

            // Use the last used profile
            if ( d->currentProfileId.isNull() ) {
                for (QString &id : ids) {
                    profilesIni.beginGroup(id);
                    if ( profilesIni.contains("Default")
                         && profilesIni.value("Default").toBool() )  {
                        d->currentProfileId = id;
                    }
                    profilesIni.endGroup();
                }
            }

            // Use the default profile
            if ( d->currentProfileId.isNull() && ids.contains("default")) {
                d->currentProfileId = "default";
            }

            // Use the first
            d->currentProfileId = ids[0];
        }
    }

    // Set the profile
    setProfile(d->currentProfileId);

    // Monitor changes to the database
    connect(&d->databaseWatcher, &QFileSystemWatcher::fileChanged,
            this, &Extension::startIndexing);
}



/** ***************************************************************************/
FirefoxBookmarks::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *FirefoxBookmarks::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Get the profiles keys
        QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);
        QStringList groups = profilesIni.childGroups();

        // Extract all profiles and names and put it in the checkbox
        QComboBox *cmb = d->widget->ui.comboBox;
        for (QString &profileId : groups) {
            profilesIni.beginGroup(profileId);

            // Use name if available else id
            if ( profilesIni.contains("Name") )
                cmb->addItem( QString("%1 (%2)").arg(profilesIni.value("Name").toString(), profileId), profileId);
            else {
                cmb->addItem(profileId, profileId);
                qWarning() << qPrintable(QString("[%1] Profile '%2' does not contain a name.").arg(Core::Extension::id, profileId));
            }

            // If the profileId match set the current item of the checkbox
            if (profileId == d->currentProfileId)
                cmb->setCurrentIndex(cmb->count() - 1);

            profilesIni.endGroup();
        }

        connect(cmb, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
                this, &Extension::setProfile);

        // Fuzzy
        QCheckBox *ckb = d->widget->ui.fuzzy;
        ckb->setChecked(d->offlineIndex.fuzzy());
        connect(ckb, &QCheckBox::clicked, this, &Extension::changeFuzzyness);

        // Which app to use
        ckb = d->widget->ui.openWithFirefox;
        ckb->setChecked(d->openWithFirefox);
        connect(ckb, &QCheckBox::clicked, this, &Extension::changeOpenPolicy);

        // Status bar
        ( d->futureWatcher.isRunning() )
            ? d->widget->ui.label_statusbar->setText("Indexing bookmarks ...")
            : d->widget->ui.label_statusbar->setText(QString("%1 bookmarks indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, d->widget->ui.label_statusbar, &QLabel::setText);

    }
    return d->widget;
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::handleQuery(Core::Query *query) {

    // Search for matches
    const vector<shared_ptr<Core::Indexable>> &indexables = d->offlineIndex.search(query->searchTerm().toLower());

    // Add results to query.
    vector<pair<shared_ptr<Core::Item>,short>> results;
    for (const shared_ptr<Core::Indexable> &item : indexables)
        results.emplace_back(std::static_pointer_cast<Core::StandardIndexItem>(item), 0);

    query->addMatches(results.begin(), results.end());
}


/** ***************************************************************************/
void FirefoxBookmarks::Extension::startIndexing() {

    // Never run concurrent
    if ( d->futureWatcher.future().isRunning() )
        return;

    // Run finishIndexing when the indexing thread finished
    d->futureWatcher.disconnect();
    connect(&d->futureWatcher, &QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>>::finished,
            this, &Extension::finishIndexing);

    // Run the indexer thread
    d->futureWatcher.setFuture(QtConcurrent::run(this, &Extension::indexFirefoxBookmarks));

    // Notification
    qDebug() << qPrintable(QString("[%1] Start indexing in background thread.").arg(Core::Extension::id));
    emit statusInfo("Indexing bookmarks ...");
}


/** ***************************************************************************/
void FirefoxBookmarks::Extension::finishIndexing() {

    // Get the thread results
    d->index = d->futureWatcher.future().result();

    // Rebuild the offline index
    d->offlineIndex.clear();
    for (const auto &item : d->index)
        d->offlineIndex.add(item);

    // Notification
    qDebug() <<  qPrintable(QString("[%1] Indexing done (%2 items).").arg(Core::Extension::id).arg(d->index.size()));
    emit statusInfo(QString("%1 bookmarks indexed.").arg(d->index.size()));
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::setProfile(const QString& profile) {

    d->currentProfileId = profile;

    QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);

    // Check if profile id is in profiles file
    if ( !profilesIni.childGroups().contains(d->currentProfileId) ){
        qWarning() << qPrintable(QString("[%1] Profile '%2' not found.").arg(Core::Extension::id, d->currentProfileId));
        return;
    }

    // Enter the group
    profilesIni.beginGroup(d->currentProfileId);

    // Check if the profile contains a path key
    if ( !profilesIni.contains("Path") ){
        qWarning() << qPrintable(QString("[%1] Profile '%2' does not contain a path.").arg(Core::Extension::id, d->currentProfileId));
        return;
    }

    // Get the correct absolute profile path
    QString profilePath = ( profilesIni.contains("IsRelative") && profilesIni.value("IsRelative").toBool())
            ? QFileInfo(d->profilesIniPath).dir().absoluteFilePath(profilesIni.value("Path").toString())
            : profilesIni.value("Path").toString();

    // Build the database path
    QString dbPath = QString("%1/places.sqlite").arg(profilePath);

    // Set the databases path
    QSqlDatabase db = QSqlDatabase::database(Core::Extension::id);
    db.setDatabaseName(dbPath);

    // Set a file system watcher on the database monitoring changes
    if (!d->databaseWatcher.files().isEmpty())
        d->databaseWatcher.removePaths(d->databaseWatcher.files());
    d->databaseWatcher.addPath(dbPath);

    startIndexing();

    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_PROFILE), d->currentProfileId);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeFuzzyness(bool fuzzy) {
    d->offlineIndex.setFuzzy(fuzzy);
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_FUZZY), fuzzy);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeOpenPolicy(bool useFirefox) {
    d->openWithFirefox = useFirefox;
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::Extension::id, CFG_USE_FIREFOX), useFirefox);
    startIndexing();
}



/** ***************************************************************************/
vector<shared_ptr<Core::StandardIndexItem>>
FirefoxBookmarks::Extension::indexFirefoxBookmarks() const {

    QSqlDatabase database = QSqlDatabase::database(Core::Extension::id);

    if (!database.open()) {
        qWarning() << qPrintable(QString("[%1] Could not open database: %2").arg(Core::Extension::id, database.databaseName()));
        return vector<shared_ptr<Core::StandardIndexItem>>();
    }

    // Build a new index
    vector<shared_ptr<StandardIndexItem>> bookmarks;

    QSqlQuery result(database);

    if ( !result.exec("SELECT b1.guid, p.title, p.url, b2.title " // id, title, url, parent
                      "FROM moz_bookmarks AS b1 "
                      "JOIN moz_bookmarks AS b2 ON b1.parent = b2.id " // attach parent names
                      "JOIN moz_places AS p  ON b1.fk = p.id " // attach title string and url
                      "WHERE b1.type = 1 AND p.title IS NOT NULL") ) { // filter bookmarks with nonempty title string
        qWarning() << qPrintable(QString("[%1] Querying bookmarks failed: %2").arg(Core::Extension::id, result.lastError().text()));
        return vector<shared_ptr<Core::StandardIndexItem>>();
    }

    while (result.next()) {

        // Url will be used more often
        QString urlstr = result.value(2).toString();

        // Create item
        shared_ptr<StandardIndexItem> ssii  = std::make_shared<StandardIndexItem>(result.value(0).toString());
        ssii->setText(result.value(1).toString());
        ssii->setSubtext(urlstr);
        ssii->setIconPath(":firefox");

        // Add severeal secondary index keywords
        vector<Indexable::WeightedKeyword> weightedKeywords;
        QUrl url(urlstr);
        QString host = url.host();
        weightedKeywords.emplace_back(ssii->text(), USHRT_MAX);
        weightedKeywords.emplace_back(host.left(host.size()-url.topLevelDomain().size()), USHRT_MAX/2);
        weightedKeywords.emplace_back(result.value(2).toString(), USHRT_MAX/4); // parent dirname
        ssii->setIndexKeywords(std::move(weightedKeywords));

        // Add actions
        vector<shared_ptr<Action>> actions;

        shared_ptr<StandardAction> actionDefault = std::make_shared<StandardAction>();
        actionDefault->setText("Open in default browser");
        actionDefault->setAction([urlstr](){
            QDesktopServices::openUrl(QUrl(urlstr));
        });

        shared_ptr<StandardAction> actionFirefox = std::make_shared<StandardAction>();
        actionFirefox->setText("Open in firefox");
        actionFirefox->setAction([urlstr, this](){
            QProcess::startDetached(d->firefoxExecutable, {urlstr});
        });

        shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
        action->setText("Copy url to clipboard");
        action->setAction([urlstr](){ QApplication::clipboard()->setText(urlstr); });

        // Set the order of the actions
        if ( d->openWithFirefox )  {
            actions.push_back(std::move(actionFirefox));
            actions.push_back(std::move(actionDefault));
        } else {
            actions.push_back(std::move(actionDefault));
            actions.push_back(std::move(actionFirefox));
        }
        actions.push_back(std::move(action));

        ssii->setActions(std::move(actions));

        bookmarks.push_back(std::move(ssii));
    }

    return bookmarks;
}
