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
#include "objects.hpp"
#include "query.h"
#include "albertapp.h"

/** ***************************************************************************/
Debug::Extension::Extension() : IExtension("org.albert.extension.debug") {
    QSettings *s = qApp->settings();
    s->beginGroup(id);
    setDelay(s->value("delay", 100).toUInt());
    setCount(s->value("count", 5).toUInt());
    setAsync(s->value("async", false).toBool());
    setTrigger(s->value("trigger", "dbg").toString());
    s->endGroup();
}



/** ***************************************************************************/
Debug::Extension::~Extension() {
    QSettings *s = qApp->settings();
    s->beginGroup(id);
    s->setValue("delay", delay());
    s->setValue("count", count());
    s->setValue("async", async());
    s->setValue("trigger", trigger());
    s->endGroup();
}



/** ***************************************************************************/
QWidget *Debug::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);

        widget_->ui.spinBox_delay->setValue(delay());
        connect(widget_->ui.spinBox_delay, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                this, &Extension::setDelay);

        widget_->ui.spinBox_count->setValue(count());
        connect(widget_->ui.spinBox_count, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                this, &Extension::setCount);

        widget_->ui.groupBox_async->setChecked(async());
        connect(widget_->ui.groupBox_async, &QGroupBox::toggled,
                this, &Extension::setAsync);

        widget_->ui.lineEdit_trigger->setText(trigger());
        connect(widget_->ui.lineEdit_trigger, &QLineEdit::textChanged,
                this, &Extension::setTrigger);


    }
    return widget_;
}



/** ***************************************************************************/
void Debug::Extension::handleQuery(Query query) {
    if (!query.isValid())
        return;
    // Avoid annoying warnings
    Q_UNUSED(query)
    for (uint i = 0 ; i < count_; ++i){

        if (async_)
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_));

        if (!query.isValid())
            return;

        std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>();
        item->setText(QString("Das Item #%1").arg(i));
        item->setSubtext(QString("Toll, das Item #%1").arg(i));
        item->setIcon("");
        item->setAction([](){});
        query.addMatch(item, 0);
    }
}


