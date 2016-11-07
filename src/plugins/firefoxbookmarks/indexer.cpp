// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
//                    2016 Martin Buergmann
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
#include <QUrl>
#include <QClipboard>
#include <QDesktopServices>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>
#include <functional>
#include <vector>
#include <memory>
#include "indexer.h"
#include "extension.h"
#include "standardobjects.h"
#include "albertapp.h"
using std::shared_ptr;
using std::vector;


/** ***************************************************************************/
void FirefoxBookmarks::Extension::Indexer::run() {

    if (!base_.open()) {
        qWarning("Could not open places.sqlite");
        return;
    }

    // Notification
    qDebug("[%s] Start indexing in background thread.", extension_->name_);
    emit statusInfo("Indexing bookmarks ...");

    // Build a new index
    vector<SharedStdIdxItem> bookmarks;

    QSqlQuery result(base_);
    // Don't knwo what type=1 does, but it only returns real bookmarks (no folders) and some trash...
    if (!result.exec("SELECT b.id,b.title,b.parent,p.url FROM moz_bookmarks b JOIN moz_places p ON b.fk = p.id WHERE b.type = 1")) {
        qWarning() << result.lastError().text();
        return;
    }

    while (result.next()) {
        QString id = result.value("id").toString();
        QString title = result.value("title").toString();
        QString urlstr = result.value("url").toString();
        QString parent = result.value("parent").toString();

        if (title.isEmpty()) continue;  // This is most likely something else

        SharedStdIdxItem ssii  = std::make_shared<StandardIndexItem>(id);
        ssii->setText(title);
        ssii->setSubtext(urlstr);
        ssii->setIconPath(":firefox");

        std::vector<IIndexable::WeightedKeyword> weightedKeywords;
        QUrl url(urlstr);
        QString host = url.host();
        weightedKeywords.emplace_back(title, USHRT_MAX);
        weightedKeywords.emplace_back(host.left(host.size()-url.topLevelDomain().size()), USHRT_MAX/2);

        // Scan the parent folders
        QSqlQuery preparedQuery(base_);
        if (preparedQuery.prepare("SELECT id,title,parent,guid FROM moz_bookmarks WHERE id = :parentid")) {
            QSqlQuery curpar(preparedQuery);
            curpar.bindValue("parentid", parent);
            while (curpar.exec()) {
                if (curpar.first()) {
                    QString guid = curpar.value("guid").toString();
                    if (guid == "root________") // This is the root element
                        break;
                    title = curpar.value("title").toString();
                    if (title.isEmpty()) {
                        curpar.bindValue("parentid", curpar.value("parent"));
                        continue;
                    }
                    weightedKeywords.emplace_back(title, USHRT_MAX/4);
                } else {
                    //qWarning("[%s:Indexer Thread] Statement yielded no result! (or broke)", extension_->name().toStdString().c_str());
                    break;
                }
            }

        } else {
            qWarning("[%s:Indexer Thread] Could not prepare statement!", extension_->name_);
        }

        ssii->setIndexKeywords(std::move(weightedKeywords));

        std::vector<SharedAction> actions;
        SharedStdAction action = std::make_shared<StandardAction>();
        bool exeDirect = extension_->firefoxExeFound_;

        if (exeDirect)
            action->setText("Open in firefox");  // If we have firefox we open ff-bookmarks in firefox
        else
            action->setText("Open in default browser"); // If the exe has another name (like iceweasel) which we didn't check for, lets assume the default browser handles this well

        action->setAction([urlstr, exeDirect](ExecutionFlags*){
            if (exeDirect)
                QProcess::startDetached("firefox", QStringList() << urlstr);
            else
                QDesktopServices::openUrl(QUrl(urlstr));
        });
        actions.push_back(std::move(action));

        action = std::make_shared<StandardAction>();
        action->setText("Copy url to clipboard");
        action->setAction([urlstr](ExecutionFlags*){
            QApplication::clipboard()->setText(urlstr);
        });
        actions.push_back(std::move(action));

        ssii->setActions(std::move(actions));

        bookmarks.push_back(std::move(ssii));

        if (abort_) {
            base_.close();
            return;
        }
    }

    base_.close();

    /*
     *  ▼ CRITICAL ▼
     */

    // Lock the access
    QMutexLocker locker(&extension_->indexAccess_);

    // Abortion requested while block
    if (abort_)
        return;

    // Set the new index (use swap to shift destruction out of critical area)
    std::swap(extension_->index_, bookmarks);

    // Rebuild the offline index
    extension_->offlineIndex_.clear();
    for (const auto &item : extension_->index_)
        extension_->offlineIndex_.add(item);

    // Notification
    qDebug("[%s] Indexing done (%d items)", extension_->name_, static_cast<int>(extension_->index_.size()));
    emit statusInfo(QString("Indexed %1 bookmarks").arg(extension_->index_.size()));
}
