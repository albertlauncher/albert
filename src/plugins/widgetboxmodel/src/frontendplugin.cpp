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

#include "frontendwidget.h"
#include "frontendplugin.h"


/** ***************************************************************************/
WidgetBoxModel::FrontendPlugin::FrontendPlugin()
    : frontendWidget_(new FrontendWidget){

    connect(frontendWidget_.get(), &FrontendWidget::inputChanged,
            this, &Frontend::inputChanged);

    connect(frontendWidget_.get(), &FrontendWidget::settingsWidgetRequested,
            this, &Frontend::settingsWidgetRequested);

    connect(frontendWidget_.get(), &FrontendWidget::widgetShown,
            this, &Frontend::widgetShown);

    connect(frontendWidget_.get(), &FrontendWidget::widgetHidden,
            this, &Frontend::widgetHidden);
}


/** ***************************************************************************/
WidgetBoxModel::FrontendPlugin::~FrontendPlugin() {
}


/** ***************************************************************************/
bool WidgetBoxModel::FrontendPlugin::isVisible() {
    return frontendWidget_->isVisible();
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendPlugin::setVisible(bool visible) {
    frontendWidget_->setVisible(visible);
}


/** ***************************************************************************/
QString WidgetBoxModel::FrontendPlugin::input() {
    return frontendWidget_->input();
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendPlugin::setInput(const QString &input) {
    frontendWidget_->setInput(input);
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendPlugin::setModel(QAbstractItemModel *m) {
    frontendWidget_->setModel(m);
}


/** ***************************************************************************/
QWidget *WidgetBoxModel::FrontendPlugin::widget(QWidget *parent) {
    return frontendWidget_->widget(parent);
}

