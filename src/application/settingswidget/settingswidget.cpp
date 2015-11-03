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
#include <QDebug>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShortcut>
#include <QDesktopWidget>
#include <QFocusEvent>
#include "settingswidget.h"
#include "hotkeymanager.h"
#include "mainwidget.h"
#include "pluginmanager.h"
#include "pluginmodel.h"
#include "iextension.h"


/** ***************************************************************************/
SettingsWidget::SettingsWidget(MainWidget *mainWidget, HotkeyManager *hotkeyManager, PluginManager *pluginManager, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), _mainWidget(mainWidget), _hotkeyManager(hotkeyManager), _pluginManager(pluginManager) {

    ui.setupUi(this);
    setWindowFlags(Qt::Window|Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);


    /*
     * NEWS
     */
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &SettingsWidget::newsReplyReceived);
    connect(manager, &QNetworkAccessManager::finished,
            manager, &SettingsWidget::deleteLater);
    manager->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/ManuelSchneid3r/albert/master/dist/changes-0.8.txt")));

    /*
     * GENERAL TAB
     */

    // HOTKEY STUFF
    QSet<int> hks = hotkeyManager->hotkeys();
    if (hks.size() < 1)
        ui.grabKeyButton_hotkey->setText("Press to set hotkey");
    else
        ui.grabKeyButton_hotkey->setText(QKeySequence(*hks.begin()).toString()); // OMG
    connect(ui.grabKeyButton_hotkey, &GrabKeyButton::keyCombinationPressed,
            this, &SettingsWidget::changeHotkey);

    // ALWAYS CENTER
    ui.checkBox_center->setChecked(mainWidget->showCentered());
    connect(ui.checkBox_center, &QCheckBox::toggled,
            mainWidget, &MainWidget::setShowCentered);

    // HIDE ON FOCUS LOSS
    ui.checkBox_hideOnFocusLoss->setChecked(mainWidget->hideOnFocusLoss());
    connect(ui.checkBox_hideOnFocusLoss, &QCheckBox::toggled,
            mainWidget, &MainWidget::setHideOnFocusLoss);

    // MAX PROPOSALS
    ui.spinBox_proposals->setValue(mainWidget->ui.proposalList->maxItems());
    connect(ui.spinBox_proposals, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
            mainWidget->ui.proposalList, &ProposalList::setMaxItems);

    // INFO BELOW ITEM
    ui.checkBox_showInfo->setChecked(mainWidget->ui.proposalList->showInfo());
    connect(ui.checkBox_showInfo, &QCheckBox::toggled,
            mainWidget->ui.proposalList, &ProposalList::setShowInfo);

    // INFO FOR UNSELECTED
    ui.checkBox_selectedOnly->setChecked(mainWidget->ui.proposalList->selectedOnly());
    connect(ui.checkBox_selectedOnly, &QCheckBox::toggled,
            mainWidget->ui.proposalList, &ProposalList::setSelectedOnly);


    // THEMES
    QFileInfoList themes;
    int i = 0 ;
    QStringList themeDirs =
            QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes",
                                      QStandardPaths::LocateDirectory);
    for (QDir d : themeDirs)
        themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
    for (QFileInfo fi : themes) {
        ui.comboBox_themes->addItem(fi.baseName(), fi.canonicalFilePath());
        if ( fi.baseName() == mainWidget->theme())
            ui.comboBox_themes->setCurrentIndex(i);
        ++i;
    }
    connect(ui.comboBox_themes, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &SettingsWidget::onThemeChanged);


    /*
     * PLUGIN  TAB
     */

    // Show the plugins. This* widget takes ownership of the model
    ui.listView_plugins->setModel(new PluginModel(_pluginManager, ui.listView_plugins));

    // Update infos when item is changed
    connect(ui.listView_plugins->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SettingsWidget::updatePluginInformations);

    connect(ui.listView_plugins->model(), &QAbstractListModel::dataChanged,
            this, &SettingsWidget::onPluginDataChanged);

    /*
     * ABOUT TAB
     */
    ui.about_text->setText(QString(ui.about_text->text()).replace("___versionstring___", qApp->applicationVersion()));
}



/** ***************************************************************************/
void SettingsWidget::updatePluginInformations(const QModelIndex & current) {
    // Hidde the placehodler text
    QLayoutItem *i = ui.widget_pluginInfos->layout()->takeAt(0);
    delete i->widget();
    delete i;

    if (_pluginManager->plugins()[current.row()]->isLoaded()){
        ui.widget_pluginInfos->layout()->addWidget(
                    dynamic_cast<IPlugin*>(_pluginManager->plugins()[current.row()]->instance())->widget()); // Takes ownership
    }
    else{
        QLabel *lbl = new QLabel("Plugin not loaded.");
        lbl->setEnabled(false);
        lbl->setAlignment(Qt::AlignCenter);
        ui.widget_pluginInfos->layout()->addWidget(lbl);
    }
}



/** ***************************************************************************/
void SettingsWidget::newsReplyReceived(QNetworkReply *reply) {
    ui.label_news->setText(QString::fromUtf8(reply->readAll()));
}



/** ***************************************************************************/
void SettingsWidget::changeHotkey(int newhk) {
    int oldhk = *_hotkeyManager->hotkeys().begin(); //TODO Make cool sharesdpointer design

    // Try to set the hotkey
    if (_hotkeyManager->registerHotkey(newhk)) {
        QString hkText(QKeySequence((newhk&~Qt::GroupSwitchModifier)).toString());//QTBUG-45568
        ui.grabKeyButton_hotkey->setText(hkText);
        QSettings().setValue("hotkey", hkText);
        _hotkeyManager->unregisterHotkey(oldhk);
    } else {
        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
        QMessageBox(QMessageBox::Critical, "Error",
                    QKeySequence(newhk).toString()
                    + " could not be registered.").exec();
    }
}



/** ***************************************************************************/
void SettingsWidget::onThemeChanged(int i) {
    // Apply and save the theme
    QString currentTheme = _mainWidget->theme();
    if (!_mainWidget->setTheme(ui.comboBox_themes->itemText(i))) {
        QMessageBox msgBox(QMessageBox::Critical, "Error", "Could not apply theme.");
        msgBox.exec();
        if (!_mainWidget->setTheme(currentTheme)) {
           qFatal("Rolling back theme failed.");
        }
    }
}



/** ***************************************************************************/
void SettingsWidget::onPluginDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
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
    if (_hotkeyManager->hotkeys().empty()) {
        QMessageBox msgBox(QMessageBox::Critical, "Error",
                           "Hotkey is invalid, please set it. Press Ok to go "\
                           "back to the settings, or press Cancel to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ) {
            ui.tabs->setCurrentIndex(0);
            show();
        }
        else
            qApp->quit();
        event->ignore();
        return;
    }
    event->accept();
}
