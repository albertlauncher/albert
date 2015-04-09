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

#include "extension.h"
#include <QDebug>
#include <QSettings>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <functional>

#include "query.h"
#include "configwidget.h"

/** ***************************************************************************/
void Extension::setPath(const QString &s)
{
    QFileInfo fi(s);
    // Only let _existing_ _files_ in
    if (!(fi.exists() && fi.isFile()))
        return;

    if(!_watcher.addPath(s)) // No clue why this should happen
        qCritical() << s <<  "could not be watched. Changes in this path will not be noticed.";

    _bookmarksFile = s;
    update();

    // And update the widget, if it is visible atm
    if (!_widget.isNull())
        _widget->ui.lineEdit_path->setText(s);
}

/** ***************************************************************************/
void Extension::restorePath()
{
    setPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
            + "/chromium/Default/Bookmarks");
}

/** ***************************************************************************/
void Extension::setFuzzy(bool b)
{
    qDebug() << "Set fuzzy search to" << b;

    _fuzzy = b;
    auto nameFunctor = [](const SharedItemPtr ip) -> QString {
        return std::static_pointer_cast<Bookmark>(ip)->_name;
    };
    if (_fuzzy)
        _search = new FuzzySearch<SharedBookmarkPtrList> (_index, nameFunctor);
    else
        _search = new PrefixSearch<SharedBookmarkPtrList> (_index, nameFunctor);
    _search->buildIndex();
}

/** ***************************************************************************/
void Extension::update()
{
    qDebug() << "Update 'ChromeBookmarks'";

    /*
     * First get a complete new index
     */

    std::function<void(const QJsonObject &json, SharedBookmarkPtrList *sbmpl)> rec_bmsearch =
            [&] (const QJsonObject &json, SharedBookmarkPtrList *sbmpl) {
        QJsonValue type = json["type"];
        if (type == QJsonValue::Undefined)
            return;
        if (type.toString() == "folder"){
            QJsonArray jarr = json["children"].toArray();
            for (const QJsonValue &i : jarr)
                rec_bmsearch(i.toObject(), sbmpl);
        }
        if (type.toString() == "url") {
            SharedBookmarkPtr b(new Bookmark(this));
            b->_name = json["name"].toString();// TODO ADD THE FOLDERS to the aliases
            b->_url  = json["url"].toString();
            b->_usage = 0;
            sbmpl->push_back(b);
        }
    };

    QFile f(_bookmarksFile);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << _bookmarksFile;
        return;
    }

    SharedBookmarkPtrList res;
    QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
    QJsonObject roots = json.value("roots").toObject();
    for (const QJsonValue &i : roots)
        if (i.isObject())
            rec_bmsearch(i.toObject(), &res);

    /*
     * Then update the usages ( Well  O(N²), but is N of relevant size? )
     */

    for (SharedBookmarkPtrList::iterator it = res.begin(); it != res.end(); ++it){
        SharedBookmarkPtrList::const_iterator cit =
                std::find_if(_index.begin(), _index.end(),
                             [=](SharedBookmarkPtr bm){return (bm->_url==(*it)->_url);});
        if (cit == _index.cend())// Does not exist
            (*it)->_usage = 0;
        else // Does exist
            (*it)->_usage = (*cit)->_usage;
    }

    // Well done now replace...
    _index = res;

    // Rebuild the search index
    _search->buildIndex();

}

/******************************************************************************/
/*                          INTERFACE IMPLEMENTATION                          */
/******************************************************************************/

/** ***************************************************************************/
QWidget *Extension::widget()
{
    if (_widget.isNull()){
        _widget = new ConfigWidget;

        // Paths
        _widget->ui.lineEdit_path->setText(_bookmarksFile);
        connect(_widget, &ConfigWidget::requestEditPath,
                this, &Extension::setPath);

        // Fuzzy
        _widget->ui.checkBox_fuzzy->setChecked(_fuzzy);
        connect(_widget->ui.checkBox_fuzzy, &QCheckBox::toggled,
                this, &Extension::setFuzzy);
    }
    return _widget;
}

/** ***************************************************************************/
void Extension::initialize()
{
    qDebug() << "Initialize extension 'ChromeBookmarks'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    /* Deserialze data */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadOnly| QIODevice::Text)) {
        qDebug() << "Deserializing from" << f.fileName();
        QTextStream in(&f);
        quint64 size;
        in >> size;
        for (quint64 i = 0; i < size; ++i) {
            SharedBookmarkPtr bm(new Bookmark(this));
            in >> bm->_url >> bm->_name >> bm->_usage;
            _index.push_back(bm);
        }
        f.close();
    } else
        qWarning() << "Could not open file: " << f.fileName();

    /* Initialize the search index */
    setFuzzy(s.value(CFG_FUZZY, CFG_FUZZY_DEF).toBool());

    /* Load path */
    QVariant v = s.value(CFG_BOOKMARKS);
    if (v.isValid() && v.canConvert(QMetaType::QString))
        setPath(v.toString());
    else
        restorePath();

    /* Keep in sync with the bookmarkfile */
    _timer.setInterval(UPDATE_TIMEOUT);
    _timer.setSingleShot(true);

    connect(&_watcher, &QFileSystemWatcher::fileChanged, [&](const QString &path){
        qDebug() << path << "changed! Starting timer";
        _timer.start();
    });

    connect(&_timer, &QTimer::timeout,[this](){
        qDebug() << "Timeout! Updating bookmarks in" << _bookmarksFile;
        // QFileSystemWatcher stops monitoring files once they have been
        // renamed or removed from disk, hence rewatch.
        if(!_watcher.addPath(_bookmarksFile))
            restorePath();
        update();
    });

    /* Get a generic favicon */
//    _favicon = QIcon::fromTheme("favorites", QIcon(":favicon"));
//    Q_INIT_RESOURCE(resources);
    _favicon = QIcon(":favicon");
//    Q_CLEANUP_RESOURCE(resources);
}

/** ***************************************************************************/
void Extension::finalize()
{
    qDebug() << "Finalize extension 'ChromeBookmarks'";
    QSettings s(QSettings::UserScope, "albert", "albert");

    /* Serialze data */
    QFile f(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + DATA_FILE);
    if (f.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "Serializing to " << f.fileName();
        QTextStream out( &f );
        out << static_cast<quint64>(_index.size());
        for (SharedBookmarkPtr bm : _index)
            out << bm->_url << bm->_name << bm->_usage;
        f.close();
    } else
        qCritical() << "Could not write to " << f.fileName();

    /* Save settings */
    s.setValue(CFG_FUZZY, _fuzzy);
    s.setValue(CFG_BOOKMARKS, _bookmarksFile);
}

/** ***************************************************************************/
void Extension::handleQuery(Query *q)
{
    q->addResults(_search->find(q->searchTerm()));
}

/** ***************************************************************************/
void Extension::action(const Bookmark& b, const Query &, Qt::KeyboardModifiers) const
{
    //QDesktopServices::openUrl(QUrl(_url));
    //QProcess::startDetached(QString("kstart --activate chromium %1").arg(QUrl(_url).toString()));
    QProcess::startDetached(QString("chromium %1").arg(QUrl(b._url).toString()));
}

/** ***************************************************************************/
QString Extension::actionText(const Bookmark& b, const Query &, Qt::KeyboardModifiers) const
{
    return QString("Visit '%1'.").arg(b._name);
}

/** ***************************************************************************/
QString Extension::titleText(const Bookmark &b, const Query &) const
{
    return b._name;
}

/** ***************************************************************************/
QString Extension::infoText(const Bookmark &b, const Query &) const
{
    return b._url;
}

/** ***************************************************************************/
const QIcon &Extension::icon(const Bookmark &) const
{
    return _favicon;
}
