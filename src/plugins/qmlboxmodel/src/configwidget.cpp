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

    // Get style dirs
    QStringList styleDirPaths = QStandardPaths::locateAll(
                QStandardPaths::AppDataLocation,
                "org.albert.frontend.boxmodel.qml", QStandardPaths::LocateDirectory);

    // Get style files
    QFileInfoList styles;
    for (const QString &styleDirPath : styleDirPaths) {
        QDirIterator it(styleDirPath, QDir::Dirs|QDir::NoDotAndDotDot);
        while ( it.hasNext() ) {
            QFileInfo styleFile(QDir(it.next()).filePath("MainComponent.qml"));
            if ( styleFile.exists() )
                styles << styleFile;
        }
    }

    // Fill the combobox
    int i = 0 ;
    ui.comboBox_style->addItem("Standard", QUrl(mainWindow_->DEF_STYLEPATH));
    for ( QFileInfo &style : styles ) {
        ui.comboBox_style->addItem(style.dir().dirName(), QUrl(style.canonicalFilePath()));
        if ( QUrl(style.canonicalFilePath()) == mainWindow_->source() )
            ui.comboBox_style->setCurrentIndex(i);
        ++i;
    }
    connect(ui.comboBox_style, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &ConfigWidget::onThemeChanged);

    // PROPERTY EDITOR
    connect(ui.toolButton_propertyEditor, &QToolButton::clicked, [this](){
        PropertyEditor pe(mainWindow_, this);
        pe.exec();
    });

    // PRESETS
    ui.comboBox_presets->clear();
    ui.comboBox_presets->addItem("Load preset...");
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
    QUrl url = ui.comboBox_style->itemData(i).toUrl();
    mainWindow_->setSource(url);

    // Fill presets
    ui.comboBox_presets->clear();
    ui.comboBox_presets->addItem("Load preset...");
    ui.comboBox_presets->addItems(mainWindow_->availablePresets());
}


/** ***************************************************************************/
void ConfigWidget::onPresetChanged(const QString &text) {
    // Remove the placeholder
    if (text != "Load preset..." && ui.comboBox_presets->itemText(0) == "Load preset...")
        ui.comboBox_presets->removeItem(0);

    // Apply the preset
    mainWindow_->setPreset(text);
}
