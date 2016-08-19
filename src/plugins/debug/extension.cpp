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

#include <QSettings>
#include <QDebug>
#include <chrono>
#include <thread>
#include "extension.h"
#include "configwidget.h"
#include "standardobjects.h"
#include "query.h"
#include "albertapp.h"

/** ***************************************************************************/
Debug::Extension::Extension() : AbstractExtension("org.albert.extension.debug") {
    QSettings *s = qApp->settings();
    s->beginGroup(id);
    setDelay(s->value("delay", 50).toInt());
    setCount(s->value("count", 100).toInt());
    setAsync(s->value("async", true).toBool());
    setTrigger(s->value("trigger", "dbg").toString());
    s->endGroup();
}



/** ***************************************************************************/
Debug::Extension::~Extension() {
}



/** ***************************************************************************/
QWidget *Debug::Extension::widget(QWidget *parent) {
    if (widget_.isNull())
        widget_ = new ConfigWidget(this, parent);
    return widget_;
}



/** ***************************************************************************/
void Debug::Extension::handleQuery(Query query) {
    if (!query.isValid())
        return;
    // Avoid annoying warnings
    Q_UNUSED(query)
    for (int i = 0 ; i < count_; ++i){

        if (async_)
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_));

        if (!query.isValid())
            return;

        std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>(QString::number(i));
        item->setText(QString("Das Item #%1").arg(i));
        item->setSubtext(QString("Toll, das Item #%1").arg(i));
        item->setIconPath(":debug");
        item->setActions(vector<SharedAction>());
        query.addMatch(item, 0);
    }
}



/** ***************************************************************************/
void Debug::Extension::setCount(const int &count){
    qApp->settings()->setValue(QString("%1/%2").arg(id, "count"), count);
    count_ = count;
}



/** ***************************************************************************/
void Debug::Extension::setAsync(bool async){
    qApp->settings()->setValue(QString("%1/%2").arg(id, "async"), async);
    async_ = async;
}



/** ***************************************************************************/
void Debug::Extension::setDelay(const int &delay) {
    qApp->settings()->setValue(QString("%1/%2").arg(id, "delay"), delay);
    delay_ = delay;
}



/** ***************************************************************************/
void Debug::Extension::setTrigger(const QString &trigger){
    qApp->settings()->setValue(QString("%1/%2").arg(id, "trigger"), trigger);
    trigger_ = trigger;
}


