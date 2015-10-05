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

#include "configwidget.h"
#include "application.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QStandardPaths>

/** ***************************************************************************/
Applications::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    connect(ui.pushButton_addPath, &QPushButton::clicked,
            this, &ConfigWidget::onButton_PathAdd);

    connect(ui.pushButton_removePath, &QPushButton::clicked,
            this, &ConfigWidget::onButton_PathRemove);

    connect(ui.pushButton_terminal, &QPushButton::clicked,
            this, &ConfigWidget::onButton_Terminal);
}



/** ***************************************************************************/
Applications::ConfigWidget::~ConfigWidget() {

}



/** ***************************************************************************/
void Applications::ConfigWidget::onButton_PathAdd() {
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Choose path"),
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    if(path.isEmpty())
        return;

    emit requestAddPath(path);
}



/** ***************************************************************************/
void Applications::ConfigWidget::onButton_PathRemove() {
    if (ui.listWidget_paths->currentItem() == nullptr)
        return;
    emit requestRemovePath(ui.listWidget_paths->currentItem()->text());
}



/** ***************************************************************************/
void Applications::ConfigWidget::onButton_Terminal() {
    QString newterm = QInputDialog::getText(
                this,
                tr("Terminal Command"),
                tr("Terminal"),
                QLineEdit::Normal,
                Applications::Application::terminal);

    if(newterm.isEmpty())
        return;
    Applications::Application::terminal = newterm;
}
