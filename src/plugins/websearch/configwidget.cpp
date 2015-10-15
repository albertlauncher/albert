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
#include <QFileDialog>
#include <QStandardPaths>

/** ***************************************************************************/
Websearch::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    ui.tableView_searches->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView_searches->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Initialize connections
    connect(ui.pushButton_new, &QPushButton::clicked,
            this, &ConfigWidget::onButton_new);

    connect(ui.pushButton_remove, &QPushButton::clicked,
            this, &ConfigWidget::onButton_remove);

    connect(ui.pushButton_setIcon, &QPushButton::clicked,
            this, &ConfigWidget::onButton_setIcon);

    connect(ui.pushButton_moveUp, &QPushButton::clicked,
            this, &ConfigWidget::onButton_moveUp);

    connect(ui.pushButton_moveDown, &QPushButton::clicked,
            this, &ConfigWidget::onButton_moveDown);

}



/** ***************************************************************************/
Websearch::ConfigWidget::~ConfigWidget() {

}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_new() {
    if (ui.tableView_searches->currentIndex().isValid())
        ui.tableView_searches->model()->insertRow(ui.tableView_searches->currentIndex().row());
    else
        ui.tableView_searches->model()->insertRow(ui.tableView_searches->model()->rowCount());
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_remove() {
    ui.tableView_searches->model()->removeRow(ui.tableView_searches->currentIndex().row());
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_moveUp() {
    ui.tableView_searches->model()->moveRows(
                QModelIndex(), ui.tableView_searches->currentIndex().row(), 1,
                QModelIndex(), ui.tableView_searches->currentIndex().row()-1);
    //          v before this (-1)
    //|..|..|..|..|XX|..|..|
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_moveDown() {
    ui.tableView_searches->model()->moveRows(
                QModelIndex(), ui.tableView_searches->currentIndex().row(), 1,
                QModelIndex(), ui.tableView_searches->currentIndex().row()+2);
    //             v before this (+2)
    //|..|..|XX|..|..|..|..|
}



/** ***************************************************************************/
void Websearch::ConfigWidget::onButton_setIcon() {
    int row = ui.tableView_searches->currentIndex().row();
    if (row < 0 || ui.tableView_searches->model()->rowCount() <= row)
        return;

    QString fileName =
            QFileDialog::getOpenFileName(
                this,
                tr("Choose icon"),
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                tr("Images (*.png *.svg"));
    if(fileName.isEmpty())
        return;

    ui.tableView_searches->model()->setData(ui.tableView_searches->currentIndex(), fileName, Qt::DecorationRole);
}
