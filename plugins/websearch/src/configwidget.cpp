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
ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    ui.tableView_searches->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView_searches->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Initialize connections
    connect(ui.pushButton_new, &QPushButton::clicked,
            this, [this](){ui.tableView_searches->model()->insertRow(ui.tableView_searches->currentIndex().row());});
    connect(ui.pushButton_remove, &QPushButton::clicked,
            this, [this](){ui.tableView_searches->model()->removeRow(ui.tableView_searches->currentIndex().row());});
    connect(ui.pushButton_setIcon, &QPushButton::clicked,
            this, &ConfigWidget::onButton_SetIcon);

}

/** ***************************************************************************/
ConfigWidget::~ConfigWidget()
{

}

/** ***************************************************************************/
void ConfigWidget::onButton_SetIcon()
{
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
