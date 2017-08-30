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

#include "frontendplugin.h"
#include "configwidget.h"
#include "mainwindow.h"


/** ***************************************************************************/
QmlBoxModel::FrontendPlugin::FrontendPlugin()
    : Frontend("org.albert.frontend.boxmodel.qml"),
      mainWindow(new MainWindow(this)){

    connect(mainWindow.get(), &MainWindow::inputChanged,
            this, &Frontend::inputChanged);

    connect(mainWindow.get(), &MainWindow::settingsWidgetRequested,
            this, &Frontend::settingsWidgetRequested);

    connect(mainWindow.get(), &MainWindow::visibilityChanged,
            this, [this](QWindow::Visibility visibility){
        emit ( visibility == QWindow::Visibility::Hidden ) ? widgetHidden() : widgetShown();
    });
}


/** ***************************************************************************/
QmlBoxModel::FrontendPlugin::~FrontendPlugin() {

}


/** ***************************************************************************/
bool QmlBoxModel::FrontendPlugin::isVisible() {
    return mainWindow->isVisible();
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setVisible(bool visible) {
    mainWindow->setVisible(visible);
    mainWindow->raise();
    mainWindow->requestActivate();
}


/** ***************************************************************************/
QString QmlBoxModel::FrontendPlugin::input() {
    return mainWindow->input();
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setInput(const QString &input) {
    mainWindow->setInput(input);
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setModel(QAbstractItemModel *m) {
    mainWindow->setModel(m);
}


/** ***************************************************************************/
QWidget *QmlBoxModel::FrontendPlugin::widget(QWidget *parent) {
    return new ConfigWidget(mainWindow.get(), parent);
}

