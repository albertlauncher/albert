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

#include <QDesktopServices>
#include <QUrl>
#include <QDataStream>
#include "searchengine.h"
#include "standardaction.hpp"
#include "standarditem.hpp"



/** ***************************************************************************/
SharedItem Websearch::SearchEngine::buildWebsearchItem(const QString &searchterm) {

    QString urlString = QString(url_).replace("%s", searchterm);
    QUrl url = QUrl(urlString);
    QString title = QString("Search '%1' in %2").arg(((searchterm.isEmpty()) ? "..." : searchterm), name_);

    std::shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
    action->setText(title);
    action->setAction([=](ExecutionFlags *){ QDesktopServices::openUrl(url); });

    std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>(name_);
    item->setText(title);
    item->setSubtext(urlString);
    item->setIcon(iconPath_);

    item->setActions({action});

    return item;
}



/** ***************************************************************************/
void Websearch::SearchEngine::serialize(QDataStream &out) {
    out << enabled_
        << url_
        << name_
        << trigger_
        << iconPath_;
}



/** ***************************************************************************/
void Websearch::SearchEngine::deserialize(QDataStream &in) {
    in >> enabled_
       >> url_
       >> name_
       >> trigger_
       >> iconPath_;
}
