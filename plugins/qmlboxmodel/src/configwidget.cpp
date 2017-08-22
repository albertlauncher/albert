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

#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>
#include <QMessageBox>
#include <QShortcut>
#include <QDesktopWidget>
#include <QFocusEvent>
#include "configwidget.h"
#include "mainwindow.h"
#include "propertyeditor.h"

/** ***************************************************************************/
ConfigWidget::ConfigWidget(MainWindow *mainWindow, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), mainWindow_(mainWindow) {

    ui.setupUi(this);

    // ALWAYS ON TOP
    ui.checkBox_onTop->setChecked(mainWindow_->alwaysOnTop());
    connect(ui.checkBox_onTop, &QCheckBox::toggled, mainWindow_, &MainWindow::setAlwaysOnTop);

    // HIDE ON FOCUS OUT
    ui.checkBox_hideOnFocusOut->setChecked(mainWindow_->hideOnFocusLoss());
    connect(ui.checkBox_hideOnFocusOut, &QCheckBox::toggled, mainWindow_, &MainWindow::setHideOnFocusLoss);

    // ALWAYS CENTER
    ui.checkBox_center->setChecked(mainWindow_->showCentered());
    connect(ui.checkBox_center, &QCheckBox::toggled, mainWindow_, &MainWindow::setShowCentered);


    /*
     *  STYLES
     */

    // Fill the combobox
    for ( const QmlStyleSpec &style : mainWindow_->availableStyles() ) {
        ui.comboBox_style->addItem(style.name, style.mainComponent);

        // Add tooltip
        ui.comboBox_style->setItemData(ui.comboBox_style->count()-1,
                                       QString("%1\nVersion: %2\nAuthor: %3")
                                       .arg(style.name, style.version, style.author),
                                       Qt::ToolTipRole);

        if ( style.mainComponent == mainWindow_->source().toString() )
            ui.comboBox_style->setCurrentIndex(ui.comboBox_style->count()-1);
    }
    connect(ui.comboBox_style, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &ConfigWidget::onThemeChanged);

    // PROPERTY EDITOR
    connect(ui.toolButton_propertyEditor, &QToolButton::clicked, [this](){
        PropertyEditor pe(mainWindow_);
        pe.exec();
    });

    // PRESETS
    ui.comboBox_presets->clear();
    ui.comboBox_presets->addItem("Choose preset...");
    ui.comboBox_presets->addItems(mainWindow_->availablePresets());
    connect(ui.comboBox_presets, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &ConfigWidget::onPresetChanged);


//    QDesktopWidget *dw = QApplication::desktop();
//    move(dw->availableGeometry(dw->screenNumber(QCursor::pos())).center()
//                -QPoint(width()/2,height()/2));
//    raise();
//    activateWindow();
}


/** ***************************************************************************/
void ConfigWidget::onThemeChanged(int i) {
    // Apply the theme
    mainWindow_->setSource(mainWindow_->availableStyles()[static_cast<size_t>(i)].mainComponent);

    // Fill presets
    ui.comboBox_presets->blockSignals(true);
    ui.comboBox_presets->clear();
    ui.comboBox_presets->addItem("Choose preset...");
    ui.comboBox_presets->addItems(mainWindow_->availablePresets());
    ui.comboBox_presets->blockSignals(false);
}


/** ***************************************************************************/
void ConfigWidget::onPresetChanged(const QString &text) {
    mainWindow_->setPreset(text);
}
