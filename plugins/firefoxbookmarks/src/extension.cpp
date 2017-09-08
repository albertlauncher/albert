// albert - a simple application launcher for linux
// Copyright (C) 2016-2017 Martin Buergmann
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
#include "extension.h"
#include "configwidget.h"
#include "core/extension.h"
#include "core/item.h"
#include "core/query.h"
#include "util/offlineindex.h"
#include "util/standardaction.h"
#include "util/standardindexitem.h"
#include "xdg/iconlookup.h"
using std::pair;
using std::shared_ptr;
using std::vector;
using namespace Core;

namespace {
const QString CFG_PROFILE = "profile";
const QString CFG_FUZZY   = "fuzzy";
const bool    DEF_FUZZY   = false;
const QString CFG_USE_FIREFOX   = "openWithFirefox";
const bool    DEF_USE_FIREFOX   = false;
const uint    UPDATE_DELAY = 60000;
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class FirefoxBookmarks::Private
{
public:
    Private(Extension *q) : q(q) {}

    Extension *q;

    bool openWithFirefox;
    QPointer<ConfigWidget> widget;
    QString firefoxExecutable;
    QString profilesIniPath;
    QString currentProfileId;
    QFileSystemWatcher databaseWatcher;

    vector<shared_ptr<Core::StandardIndexItem>> index;
    Core::OfflineIndex offlineIndex;

    QTimer updateDelayTimer;
    void startIndexing();
    void finishIndexing();
    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;
    std::vector<std::shared_ptr<Core::StandardIndexItem>> indexFirefoxBookmarks() const;
};


/** ***************************************************************************/
void FirefoxBookmarks::Private::startIndexing() {

    // Never run concurrent
    if ( futureWatcher.future().isRunning() )
        return;

    // Run finishIndexing when the indexing thread finished
    futureWatcher.disconnect();
    QObject::connect(&futureWatcher, &QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>>::finished,
                     std::bind(&Private::finishIndexing, this));

    // Run the indexer thread
    futureWatcher.setFuture(QtConcurrent::run(this, &Private::indexFirefoxBookmarks));

    // Notification
    qInfo() << "Start indexing Firefox bookmarks.";
    emit q->statusInfo("Indexing bookmarks ...");
}


/** ***************************************************************************/
void FirefoxBookmarks::Private::finishIndexing() {

    // Get the thread results
    index = futureWatcher.future().result();

    // Rebuild the offline index
    offlineIndex.clear();
    for (const auto &item : index)
        offlineIndex.add(item);

    // Notification
    qInfo() <<  qPrintable(QString("Indexed %1 Firefox bookmarks.").arg(index.size()));
    emit q->statusInfo(QString("%1 bookmarks indexed.").arg(index.size()));
}



/** ***************************************************************************/
vector<shared_ptr<Core::StandardIndexItem>>
FirefoxBookmarks::Private::indexFirefoxBookmarks() const {

    QSqlDatabase database = QSqlDatabase::database(q->Core::Plugin::id());

    if (!database.open()) {
        qWarning() << qPrintable(QString("Could not open Firefox database: %1").arg(database.databaseName()));
        return vector<shared_ptr<Core::StandardIndexItem>>();
    }

    // Build a new index
    vector<shared_ptr<StandardIndexItem>> bookmarks;

    QSqlQuery result(database);

    if ( !result.exec("SELECT b.guid, b.title, p.url "
                      "FROM moz_bookmarks b "
                      "JOIN moz_places p ON b.fk = p.id " // attach place (which has the url)
                      "WHERE p.url NOT LIKE 'place%'") ) {  // Those with place:... will not work with xdg-open
        qWarning() << qPrintable(QString("Querying Firefox bookmarks failed: %1").arg(result.lastError().text()));
        return vector<shared_ptr<Core::StandardIndexItem>>();
    }

    // Find an appropriate icon
    QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});
    icon = icon.isEmpty() ? ":favicon" : icon;

    while (result.next()) {

        // Url will be used more often
        QString urlstr = result.value(2).toString();

        // Create item
        shared_ptr<StandardIndexItem> ssii  = std::make_shared<StandardIndexItem>(result.value(0).toString());
        ssii->setText(result.value(1).toString());
        ssii->setSubtext(urlstr);
        ssii->setIconPath(icon);

        // Add severeal secondary index keywords
        vector<IndexableItem::IndexString> indexStrings;
        QUrl url(urlstr);
        QString host = url.host();
        indexStrings.emplace_back(ssii->text(), UINT_MAX);
        indexStrings.emplace_back(host.left(host.size()-url.topLevelDomain().size()), UINT_MAX/2);
        indexStrings.emplace_back(result.value(2).toString(), UINT_MAX/4); // parent dirname
        ssii->setIndexKeywords(std::move(indexStrings));

        // Add actions
        vector<shared_ptr<Action>> actions;

        shared_ptr<StandardAction> actionDefault = std::make_shared<StandardAction>();
        actionDefault->setText("Open URL in your default browser");
        actionDefault->setAction([urlstr](){
            QDesktopServices::openUrl(QUrl(urlstr));
        });

        shared_ptr<StandardAction> actionFirefox = std::make_shared<StandardAction>();
        actionFirefox->setText("Open URL in Firefox");
        actionFirefox->setAction([urlstr, this](){
            QProcess::startDetached(firefoxExecutable, {urlstr});
        });

        shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
        action->setText("Copy url to clipboard");
        action->setAction([urlstr](){ QApplication::clipboard()->setText(urlstr); });

        // Set the order of the actions
        if ( openWithFirefox )  {
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



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
FirefoxBookmarks::Extension::Extension()
    : Core::Extension("org.albert.extension.firefoxbookmarks"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private(this)){

    // Add a sqlite database connection for this extension, check requirements
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", Core::Plugin::id());
    if ( !db.isValid() )
        throw "Firefox executable not found.";
    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
        throw "Firefox executable not found.";

    // Find firefox executable
    d->firefoxExecutable = QStandardPaths::findExecutable("firefox");
    if (d->firefoxExecutable.isEmpty())
        throw "Firefox executable not found.";

    // Locate profiles ini
    d->profilesIniPath = QStandardPaths::locate(QStandardPaths::HomeLocation,
                                                 ".mozilla/firefox/profiles.ini",
                                                 QStandardPaths::LocateFile);
    if (d->profilesIniPath.isEmpty()) // Try a windowsy approach
        d->profilesIniPath = QStandardPaths::locate(QStandardPaths::DataLocation,
                                                     "Mozilla/firefox/profiles.ini",
                                                     QStandardPaths::LocateFile);
    if (d->profilesIniPath.isEmpty())
        throw "Could not locate profiles.ini.";

    // Load the settings
    d->currentProfileId = settings().value(CFG_PROFILE).toString();
    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->openWithFirefox = settings().value(CFG_USE_FIREFOX, DEF_USE_FIREFOX).toBool();

    // If the id does not exist find a proper default
    QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);
    if ( !profilesIni.contains(d->currentProfileId) ){

        d->currentProfileId = QString();

        QStringList ids = profilesIni.childGroups();
        if ( ids.isEmpty() )
            qWarning() << "No Firefox profiles found.";
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

    // Delay the indexing to avoid excessice resource consumption
    d->updateDelayTimer.setInterval(UPDATE_DELAY);
    d->updateDelayTimer.setSingleShot(true);

    // If the database changed, trigger the update delay
    connect(&d->databaseWatcher, &QFileSystemWatcher::fileChanged,
            &d->updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));

    // If the update delay passed, update the index
    connect(&d->updateDelayTimer, &QTimer::timeout,
            std::bind(&Private::startIndexing, d.get()));
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
                qWarning() << qPrintable(QString("Firefox profile '%1' does not contain a name.").arg(profileId));
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
void FirefoxBookmarks::Extension::handleQuery(Core::Query *query) const {

    if ( query->searchTerm().isEmpty() )
        return;

    // Search for matches
    const vector<shared_ptr<Core::IndexableItem>> &indexables = d->offlineIndex.search(query->searchTerm().toLower());

    // Add results to query.
    vector<pair<shared_ptr<Core::Item>,uint>> results;
    for (const shared_ptr<Core::IndexableItem> &item : indexables)
        results.emplace_back(std::static_pointer_cast<Core::StandardIndexItem>(item), 0);

    query->addMatches(std::make_move_iterator(results.begin()),
                      std::make_move_iterator(results.end()));
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::setProfile(const QString& profile) {

    d->currentProfileId = profile;

    QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);

    // Check if profile id is in profiles file
    if ( !profilesIni.childGroups().contains(d->currentProfileId) ){
        qWarning() << qPrintable(QString("Firefox user profile '%2' not found.").arg(d->currentProfileId));
        return;
    }

    // Enter the group
    profilesIni.beginGroup(d->currentProfileId);

    // Check if the profile contains a path key
    if ( !profilesIni.contains("Path") ){
        qWarning() << qPrintable(QString("Firefox profile '%2' does not contain a path.").arg(d->currentProfileId));
        return;
    }

    // Get the correct absolute profile path
    QString profilePath = ( profilesIni.contains("IsRelative") && profilesIni.value("IsRelative").toBool())
            ? QFileInfo(d->profilesIniPath).dir().absoluteFilePath(profilesIni.value("Path").toString())
            : profilesIni.value("Path").toString();

    // Build the database path
    QString dbPath = QString("%1/places.sqlite").arg(profilePath);

    // Set the databases path
    QSqlDatabase db = QSqlDatabase::database(Core::Plugin::id());
    db.setDatabaseName(dbPath);

    // Set a file system watcher on the database monitoring changes
    if (!d->databaseWatcher.files().isEmpty())
        d->databaseWatcher.removePaths(d->databaseWatcher.files());
    d->databaseWatcher.addPath(dbPath);

    d->startIndexing();

    settings().setValue(CFG_PROFILE, d->currentProfileId);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeFuzzyness(bool fuzzy) {
    d->offlineIndex.setFuzzy(fuzzy);
    settings().setValue(CFG_FUZZY, fuzzy);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeOpenPolicy(bool useFirefox) {
    d->openWithFirefox = useFirefox;
    settings().setValue(CFG_USE_FIREFOX, useFirefox);
    d->startIndexing();
}
