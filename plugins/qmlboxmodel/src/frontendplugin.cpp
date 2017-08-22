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
#include "mainwindow.h"
#include "configwidget.h"

class QmlBoxModel::FrontendPluginPrivate
{
public:
    MainWindow mainWindow;
};

/** ***************************************************************************/
QmlBoxModel::FrontendPlugin::FrontendPlugin()
    : d(new FrontendPluginPrivate){

    connect(&d->mainWindow, &MainWindow::inputChanged,
            this, &Frontend::inputChanged);

    connect(&d->mainWindow, &MainWindow::settingsWidgetRequested,
            this, &Frontend::settingsWidgetRequested);

    connect(&d->mainWindow, &MainWindow::visibilityChanged,
            this, [this](QWindow::Visibility visibility){
        emit ( visibility == QWindow::Visibility::Hidden ) ? widgetHidden() : widgetShown();
    });
}


/** ***************************************************************************/
QmlBoxModel::FrontendPlugin::~FrontendPlugin() {

}


/** ***************************************************************************/
bool QmlBoxModel::FrontendPlugin::isVisible() {
    return d->mainWindow.isVisible();
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setVisible(bool visible) {
    d->mainWindow.setVisible(visible);
    d->mainWindow.raise();
    d->mainWindow.requestActivate();
}


/** ***************************************************************************/
QString QmlBoxModel::FrontendPlugin::input() {
    return d->mainWindow.input();
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setInput(const QString &input) {
    d->mainWindow.setInput(input);
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setModel(QAbstractItemModel *m) {
    d->mainWindow.setModel(m);
}


/** ***************************************************************************/
QWidget *QmlBoxModel::FrontendPlugin::widget(QWidget *parent) {
    return new ConfigWidget(&d->mainWindow, parent);
}

