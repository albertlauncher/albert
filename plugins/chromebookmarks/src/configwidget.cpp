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

#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>
#include <QMessageBox>
#include "configwidget.h"

/** ***************************************************************************/
ChromeBookmarks::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.pushButton_editPath, &QPushButton::clicked,
            this, &ConfigWidget::onButton_EditPath);
}

/** ***************************************************************************/
ChromeBookmarks::ConfigWidget::~ConfigWidget()
{

}

/** ***************************************************************************/
void ChromeBookmarks::ConfigWidget::onButton_EditPath()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if(path.isEmpty()) return;
    emit requestEditPath(path);
}
