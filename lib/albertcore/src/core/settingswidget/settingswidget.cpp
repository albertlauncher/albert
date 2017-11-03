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

#include <QApplication>
#include <QCloseEvent>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDesktopWidget>
#include <QFocusEvent>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <vector>
#include <memory>
#include <utility>
#include "core_globals.h"
#include "extension.h"
#include "extensionmanager.h"
#include "frontend.h"
#include "frontendmanager.h"
#include "globalshortcut/hotkeymanager.h"
#include "loadermodel.h"
#include "pluginspec.h"
#include "querymanager.h"
#include "settingswidget.h"
#include "usagedatabase.h"
#include "trayicon.h"
using namespace std;
using namespace Core;
using namespace GlobalShortcut;

namespace {
const char* CFG_TERM = "terminal";
const char* DEF_TERM = "xterm -e";
}

EXPORT_CORE QString terminalCommand;


/** ***************************************************************************/
Core::SettingsWidget::SettingsWidget(ExtensionManager *extensionManager,
                                     FrontendManager *frontendManager,
                                     QueryManager *queryManager,
                                     HotkeyManager *hotkeyManager,
                                     TrayIcon *systemTrayIcon,
                                     QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      extensionManager_(extensionManager),
      frontendManager_(frontendManager),
      queryManager_(queryManager),
      hotkeyManager_(hotkeyManager),
      trayIcon_(systemTrayIcon) {

    ui.setupUi(this);


    /*
     * GENERAL
     */

    // HOTKEY
    QSet<int> hks = hotkeyManager->hotkeys();
    if (hks.size() < 1)
        ui.grabKeyButton_hotkey->setText("Press to set hotkey");
    else
        ui.grabKeyButton_hotkey->setText(QKeySequence(*hks.begin()).toString()); // OMG
    connect(ui.grabKeyButton_hotkey, &GrabKeyButton::keyCombinationPressed,
            this, &SettingsWidget::changeHotkey);

    // TRAY
    ui.checkBox_showTray->setChecked(trayIcon_->isVisible());
    connect(ui.checkBox_showTray, &QCheckBox::toggled,
            trayIcon_, &TrayIcon::setVisible);

    // INCREMENTAL SORT
    ui.checkBox_incrementalSort->setChecked(queryManager_->incrementalSort());
    connect(ui.checkBox_incrementalSort, &QCheckBox::toggled,
            queryManager_, &QueryManager::setIncrementalSort);

    // FRONTEND
    for ( const unique_ptr<PluginSpec> &plugin : frontendManager_->frontendSpecs() ){

        // Add item (text and id)
        ui.comboBox_frontend->addItem(plugin->name(), plugin->id());

        // Add tooltip
        ui.comboBox_frontend->setItemData(ui.comboBox_frontend->count()-1,
                                          QString("%1\nID: %2\nVersion: %3\nAuthor: %4\nDependencies: %5")
                                          .arg(plugin->name(),
                                               plugin->id(),
                                               plugin->version(),
                                               plugin->author(),
                                               plugin->dependencies().join(", ")),
                                          Qt::ToolTipRole);
        // Set to current if ids match
        if ( plugin->id() == frontendManager_->currentFrontend()->id() )
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }

    ui.tabGeneral->layout()->addWidget(frontendManager_->currentFrontend()->widget(ui.tabGeneral));

    connect(ui.comboBox_frontend, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this](int i){
        QString id = ui.comboBox_frontend->itemData(i, Qt::UserRole).toString();
        frontendManager_->setCurrentFrontend(id);

        QLayoutItem* item;
        for ( int i = ui.tabGeneral->layout()->count() - 1; i > 0; --i ) {
            item = ui.tabGeneral->layout()->takeAt(i);
            delete item->widget();
            delete item;
        }

        ui.tabGeneral->layout()->addWidget(frontendManager_->currentFrontend()->widget(ui.tabGeneral));

    });


    // TERM CMD (TOOOOOOOOOOODOOOOOOOOOO CENTRALIZE THIS)
    // Define the (global extern) terminal command
    terminalCommand = QSettings(qApp->applicationName()).value(CFG_TERM, DEF_TERM).toString();

    // Available terms
    std::vector<std::pair<QString, QString>> terms {
        {"Cool Retro Term", "cool-retro-term -e"},
        {"Gnome Terminal", "gnome-terminal -x"},
        {"Konsole", "konsole -e"},
        {"LXTerminal", "lxterminal -e"},
        {"Mate-Termial", "mate-terminal -x"},
        {"RoxTerm", "roxterm -x"},
        {"Terminator", "terminator -x"},
        {"urxvt", "urxvt -e"},
        {"UXTerm", "uxterm -e"},
        {"XFCE-Terminal", "xfce4-terminal -x"},
        {"XTerm", "xterm -e"}
    };

    // Fill checkbox
    for ( ulong i = 0; i < terms.size(); ++i ) {
        if ( !QStandardPaths::findExecutable(terms[i].second.split(' ').first()).isEmpty() ){
            ui.comboBox_term->addItem(terms[i].first, terms[i].second);
            if ( terms[i].second == terminalCommand )
                ui.comboBox_term->setCurrentIndex(static_cast<int>(ui.comboBox_term->count()-1));
        }
    }
    ui.comboBox_term->insertSeparator(ui.comboBox_term->count());
    ui.comboBox_term->addItem(tr("Custom"));

    ui.lineEdit_term->setText(terminalCommand);
    ui.lineEdit_term->setEnabled(ui.comboBox_term->currentIndex() == ui.comboBox_term->count()-1);
    connect(ui.comboBox_term, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this](int index){
        if ( index != ui.comboBox_term->count()-1) {
            terminalCommand = ui.comboBox_term->currentData(Qt::UserRole).toString();
            QSettings(qApp->applicationName()).setValue(CFG_TERM, terminalCommand);
        }
        ui.lineEdit_term->setEnabled(index == ui.comboBox_term->count()-1);
        ui.lineEdit_term->setText(terminalCommand);
    });

    connect(ui.lineEdit_term, &QLineEdit::textEdited, [](QString str){
        terminalCommand = str;
        QSettings(qApp->applicationName()).setValue(CFG_TERM, terminalCommand);
    });

    // Cache
    connect(ui.pushButton_clearHistory, &QPushButton::clicked, &UsageDatabase::clearRecentlyUsed);

    /*
     * PLUGINS
     */

    // Show the plugins. This* widget takes ownership of the model
    ui.listView_plugins->setModel(new LoaderModel(extensionManager_, ui.listView_plugins));

    // Update infos when item is changed
    connect(ui.listView_plugins->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SettingsWidget::updatePluginInformations);

    connect(ui.listView_plugins->model(), &QAbstractListModel::dataChanged,
            this, &SettingsWidget::onPluginDataChanged);

    // Initially hide the title label
    ui.label_pluginTitle->hide();


    /*
     * ABOUT
     */

    ui.about_text->setText(QString(ui.about_text->text()).replace("___versionstring___", qApp->applicationVersion()));

    QDesktopWidget *dw = QApplication::desktop();
    move(dw->availableGeometry(dw->screenNumber(QCursor::pos())).center()
                -QPoint(width()/2,height()/2));
    raise();
    activateWindow();
}



/** ***************************************************************************/
void SettingsWidget::updatePluginInformations(const QModelIndex & current) {
    // Hidde the placehodler text
    QLayoutItem *i = ui.widget_pluginInfos->layout()->takeAt(1);
    delete i->widget();
    delete i;

    if ( extensionManager_->extensionSpecs()[static_cast<size_t>(current.row())]->state()
         == PluginSpec::State::Loaded ){
        Extension *extension = dynamic_cast<Extension*>(
                    extensionManager_->extensionSpecs()[static_cast<size_t>(current.row())]->instance());
        if (!extension){
            qWarning() << "Cannot cast an object of extension spec to an extension!";
            return; // Should no happen
        }
        QWidget *pw = extension->widget();
        if ( pw->layout() != nullptr)
            pw->layout()->setMargin(0);
        ui.widget_pluginInfos->layout()->addWidget(pw);// Takes ownership
        ui.label_pluginTitle->setText(extension->name());
        ui.label_pluginTitle->show();
    }
    else{
        QString msg("Plugin not loaded.\n%1");
        QLabel *lbl = new QLabel(msg.arg(extensionManager_->extensionSpecs()[static_cast<size_t>(current.row())]->lastError()));
        lbl->setEnabled(false);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setWordWrap(true);
        ui.widget_pluginInfos->layout()->addWidget(lbl);
        ui.label_pluginTitle->hide();
    }
}



/** ***************************************************************************/
void SettingsWidget::changeHotkey(int newhk) {
    int oldhk = *hotkeyManager_->hotkeys().begin(); //TODO Make cool sharesdpointer design

    // Try to set the hotkey
    if (hotkeyManager_->registerHotkey(newhk)) {
        QString hkText(QKeySequence((newhk&~Qt::GroupSwitchModifier)).toString());//QTBUG-45568
        ui.grabKeyButton_hotkey->setText(hkText);
        QSettings(qApp->applicationName()).setValue("hotkey", hkText);
        hotkeyManager_->unregisterHotkey(oldhk);
    } else {
        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
        QMessageBox(QMessageBox::Critical, "Error",
                    QKeySequence(newhk).toString() + " could not be registered.",
                    QMessageBox::NoButton,
                    this).exec();
    }
}



/** ***************************************************************************/
void SettingsWidget::onPluginDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
    Q_UNUSED(bottomRight)
    if (topLeft == ui.listView_plugins->currentIndex())
        for (int role : roles)
            if (role == Qt::CheckStateRole)
                updatePluginInformations(topLeft);
}



/** ***************************************************************************/
void SettingsWidget::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape ) {
        close();
    }
}



/** ***************************************************************************/
void SettingsWidget::closeEvent(QCloseEvent *event) {
    if (hotkeyManager_->hotkeys().empty()) {
        QMessageBox msgBox(QMessageBox::Warning, "Hotkey Missing",
                           "Hotkey is invalid, please set it. Press OK to go "\
                           "back to the settings.",
                           QMessageBox::Ok|QMessageBox::Ignore,
                           this);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ) {
            ui.tabs->setCurrentIndex(0);
            show();
            event->ignore();
            return;
        }
    }
    event->accept();
}
