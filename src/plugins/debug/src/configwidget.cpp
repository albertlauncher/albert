// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include <QSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include "configwidget.h"

Debug::ConfigWidget::ConfigWidget(Extension * extension, QWidget * parent)
    : QWidget(parent), extension_(extension)
{
    ui.setupUi(this);
    ui.spinBox_delay->setValue(extension_->delay());
    connect(ui.spinBox_delay, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            extension_, &Extension::setDelay);

    ui.spinBox_count->setValue(extension_->count());
    connect(ui.spinBox_count, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            extension_, &Extension::setCount);

    ui.groupBox_async->setChecked(extension_->async());
    connect(ui.groupBox_async, &QGroupBox::toggled,
            extension_, &Extension::setAsync);

    ui.lineEdit_trigger->setText(extension_->trigger());
    connect(ui.lineEdit_trigger, &QLineEdit::textChanged,
            extension_, &Extension::setTrigger);
}
