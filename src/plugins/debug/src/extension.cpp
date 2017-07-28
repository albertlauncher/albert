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

#include <QApplication>
#include <QDebug>
#include <QPointer>
#include <QSettings>
#include <chrono>
#include <thread>
#include "configwidget.h"
#include "extension.h"
#include "core/query.h"
#include "util/standarditem.h"
using Core::StandardItem;


class Debug::Private
{
public:
    QPointer<ConfigWidget> widget;
    int delay;
    int count;
    bool async;
    QString trigger;
};




/** ***************************************************************************/
Debug::Extension::Extension()
    : Core::Extension("org.albert.extension.debug"),
      Core::QueryHandler(Core::Extension::id),
      d(new Private) {
    QSettings s(qApp->applicationName());
    s.beginGroup(Core::QueryHandler::id);
    d->delay = s.value("delay", 50).toInt();
    d->count = s.value("count", 100).toInt();
    d->async = s.value("async", true).toBool();
    d->trigger = s.value("trigger", "dbg").toString();
    s.endGroup();
}



/** ***************************************************************************/
Debug::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Debug::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(this, parent);
    return d->widget;
}



/** ***************************************************************************/
QStringList Debug::Extension::triggers() const {
    return {d->trigger};
}



/** ***************************************************************************/
const QString& Debug::Extension::trigger() const {
    return d->trigger;
}



/** ***************************************************************************/
void Debug::Extension::handleQuery(Core::Query * query) {

    // This extension must run only triggered
    if ( !query->isTriggered() )
        return;

    for (int i = 0 ; i < d->count; ++i){

        if (d->async)
            std::this_thread::sleep_for(std::chrono::milliseconds(d->delay));

        if (!query->isValid())
            return;

        std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>(QString::number(i));
        item->setText(QString("Das Item #%1").arg(i));
        item->setSubtext(QString("Toll, das Item #%1").arg(i));
        item->setIconPath(":debug");
        query->addMatch(item, 0);
    }
}



/** ***************************************************************************/
bool Debug::Extension::isLongRunning() const {
    return d->async;
}



/** ***************************************************************************/
int Debug::Extension::count() const{
    return d->count;
}



/** ***************************************************************************/
void Debug::Extension::setCount(const int &count){
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::QueryHandler::id, "count"), count);
    d->count = count;
}



/** ***************************************************************************/
bool Debug::Extension::async() const{
    return d->async;
}



/** ***************************************************************************/
void Debug::Extension::setAsync(bool async){
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::QueryHandler::id, "async"), async);
    d->async = async;
}



/** ***************************************************************************/
int Debug::Extension::delay() const {
    return d->delay;
}



/** ***************************************************************************/
void Debug::Extension::setDelay(const int &delay) {
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::QueryHandler::id, "delay"), delay);
    d->delay = delay;
}



/** ***************************************************************************/
void Debug::Extension::setTrigger(const QString &trigger){
    QSettings(qApp->applicationName()).setValue(QString("%1/%2").arg(Core::QueryHandler::id, "trigger"), trigger);
    d->trigger = trigger;
}


