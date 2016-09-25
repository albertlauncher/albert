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


/** ***************************************************************************/
Websearch::SearchEngine::SearchEngine(QString name, QString url, QString trigger, QString iconPath, bool enabled)
    : enabled_(enabled), name_(name), url_(url), trigger_(trigger), iconPath_(iconPath) {

}



/** ***************************************************************************/
QString Websearch::SearchEngine::text(const QString &query) const {
    return QString("Search '%1' in %2").arg(((query.isEmpty()) ? "..." : query), name_);
}



/** ***************************************************************************/
QString Websearch::SearchEngine::subtext(const QString &query) const {
    return QString(url_).replace("%s", query);
}



/** ***************************************************************************/
QString Websearch::SearchEngine::iconPath() const {
    return iconPath_;
}



/** ***************************************************************************/
void Websearch::SearchEngine::activate(ExecutionFlags *, const QString &query) {
    QDesktopServices::openUrl(QUrl(QString(url_).replace("%s", query)));
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
