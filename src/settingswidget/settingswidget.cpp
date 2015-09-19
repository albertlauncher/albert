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
#include "interfaces/iextension.h"


/** ***************************************************************************/
SettingsWidget::SettingsWidget(MainWidget *mainWidget, HotkeyManager *hotkeyManager, PluginManager *pluginManager, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), _mainWidget(mainWidget), _hotkeyManager(hotkeyManager), _pluginManager(pluginManager) {

    ui.setupUi(this);
    setWindowFlags(Qt::Window|Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);


    /*
     * GENERAL TAB
     */

    // HOTKEY STUFF
    QSet<int> hks = hotkeyManager->hotkeys();
    if (hks.size() < 1)
        ui.grabKeyButton_hotkey->setText("Press to set hotkey");
    else
        ui.grabKeyButton_hotkey->setText(QKeySequence(*hks.begin()).toString()); // OMG
    connect(ui.grabKeyButton_hotkey, &GrabKeyButton::clicked, hotkeyManager, &HotkeyManager::disable);
    connect(ui.grabKeyButton_hotkey, &GrabKeyButton::keyCombinationPressed, this, &SettingsWidget::changeHotkey);


    // ALWAYS CENTER
    ui.checkBox_center->setChecked(mainWidget->showCenterd());
    connect(ui.checkBox_center, &QCheckBox::toggled, mainWidget, &MainWidget::setShowCentered);
    // MAX PROPOSALS
    ui.spinBox_proposals->setValue(mainWidget->ui.proposalList->maxItems());
    connect(ui.spinBox_proposals, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, mainWidget->ui.proposalList, &ProposalList::setMaxItems);
    // INFO BELOW ITEM
    ui.checkBox_showInfo->setChecked(mainWidget->ui.proposalList->showInfo());
    connect(ui.checkBox_showInfo, &QCheckBox::toggled, mainWidget->ui.proposalList, &ProposalList::setShowInfo);
    // INFO FOR UNSELECTED
    ui.checkBox_selectedOnly->setChecked(mainWidget->ui.proposalList->selectedOnly());
    connect(ui.checkBox_selectedOnly, &QCheckBox::toggled, mainWidget->ui.proposalList, &ProposalList::setSelectedOnly);


    // THEMES
    QFileInfoList themes;
    int i = 0 ;
    QStringList themeDirs =
            QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes",
                                      QStandardPaths::LocateDirectory);
    for (QDir d : themeDirs)
        themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
    for (QFileInfo fi : themes){
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

    // PLUGIN  LIST
    updatePluginList();
    ui.treeWidget_plugins->expandAll();
    ui.treeWidget_plugins->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.treeWidget_plugins->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect(ui.treeWidget_plugins, &QTreeWidget::currentItemChanged,
            this, &SettingsWidget::updatePluginInformations);

    // Blacklist items if the checbox is cklicked
    connect(ui.treeWidget_plugins, &QTreeWidget::itemChanged, this, &SettingsWidget::onPluginItemChanged);


    connect(ui.pushButton_pluginHelp, &QPushButton::clicked,
            this, &SettingsWidget::openPluginHelp);

    connect(ui.pushButton_pluginConfig, &QPushButton::clicked,
            this, &SettingsWidget::openPluginConfig);

    connect(ui.treeWidget_plugins, &QTreeWidget::itemDoubleClicked,
            this, &SettingsWidget::openPluginConfig);
}



/** ***************************************************************************/
SettingsWidget::~SettingsWidget() {
}



/** ***************************************************************************/
void SettingsWidget::openPluginHelp() {
//    if (ui.treeWidget_plugins->currentIndex().isValid()){
//        QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
//        for (PluginLoader *plugin : _pluginManager->plugins()){
//            if (plugin->fileName() == path) { // MUST HAPPEN
//                QWidget *w = dynamic_cast<IPlugin*>(plugin->instance())->widget();
//                w->setParent(this);
//                w->setWindowTitle("Plugin help");
//                w->setWindowFlags(Qt::Window|Qt::WindowCloseButtonHint);
//                w->setAttribute(Qt::WA_DeleteOnClose);
//                new QShortcut(Qt::Key_Escape, w, SLOT(close()));
//                w->move(w->parentWidget()->window()->frameGeometry().topLeft() +
//                        w->parentWidget()->window()->rect().center() -
//                        w->rect().center());
//                w->show();
//            }
//        }
//    }
}



/** ***************************************************************************/
void SettingsWidget::openPluginConfig() {
    // If the corresponding plugin is loaded open preferences
    if (ui.treeWidget_plugins->currentIndex().isValid()){
        QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
        if (_pluginManager->plugins().contains(path)
                && _pluginManager->plugins()[path]->status() == PluginLoader::Status::Loaded){
            QWidget *w = dynamic_cast<IPlugin*>(_pluginManager->plugins()[path]->instance())->widget();
            w->setParent(this);
            w->setWindowTitle("Plugin configuration");
            w->setWindowFlags(Qt::Window|Qt::WindowCloseButtonHint);
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->setWindowModality(Qt::ApplicationModal);
            new QShortcut(Qt::Key_Escape, w, SLOT(close()));
            w->move(w->parentWidget()->window()->frameGeometry().topLeft() +
                    w->parentWidget()->window()->rect().center() -
                    w->rect().center());
            w->show();
        }
    }
}



/** ***************************************************************************/
void SettingsWidget::onPluginItemChanged(QTreeWidgetItem *item, int column) {
    if ( static_cast<bool>(item->checkState(column)) )
        _pluginManager->enable(item->data(0, Qt::UserRole).toString());
    else
        _pluginManager->disable(item->data(0, Qt::UserRole).toString());
    ui.label_restart->setVisible(true);
}



/** ***************************************************************************/
void SettingsWidget::updatePluginList() {
    const QMap<QString, PluginLoader*>& plugins = _pluginManager->plugins();
    for (const PluginLoader* plugin : plugins){

        // Get the top level item of this group, make sure it exists
        QList<QTreeWidgetItem *> tlis = ui.treeWidget_plugins->findItems(plugin->group(), Qt::MatchExactly);
        if (tlis.size() > 1) // MUST NOT HAPPEN
            qCritical() << "Found multiple identical TopLevelItems in PluginList";
        QTreeWidgetItem *tli;
        if (tlis.size() == 0){
            tli= new QTreeWidgetItem({plugin->group()});
            tli->setFlags(Qt::ItemIsEnabled);
            ui.treeWidget_plugins->addTopLevelItem(tli);
        }
        else
            tli = tlis.first();

        // Create the child and insert a childitem
        QTreeWidgetItem *child = new QTreeWidgetItem();
        child->setChildIndicatorPolicy(QTreeWidgetItem::QTreeWidgetItem::DontShowIndicatorWhenChildless);
        child->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        child->setData(0, Qt::UserRole, plugin->fileName());
        child->setData(0, Qt::DisplayRole, plugin->name());
        child->setData(0, Qt::ToolTipRole, plugin->description());
        child->setData(1, Qt::ToolTipRole, "Check to load at boot");
        child->setCheckState(1, (_pluginManager->isEnabled(plugin->fileName()))?Qt::Checked:Qt::Unchecked);
        switch (plugin->status()) {
        case PluginLoader::Status::Loaded:
            child->setData(0, Qt::DecorationRole, QIcon(":plugin_loaded"));
            break;
        case PluginLoader::Status::NotLoaded:
            child->setData(0, Qt::DecorationRole, QIcon(":plugin_notloaded"));
            break;
        case PluginLoader::Status::Error:
            child->setData(0, Qt::DecorationRole, QIcon(":plugin_error"));
            break;
        default:
            qCritical() << "Unhandled PluginSpec::Status";
        }
        tli->addChild(child);
    }
}



/** ***************************************************************************/
void SettingsWidget::updatePluginInformations() {
    // Respect the toplevel items
    bool isTopLevelItem = !ui.treeWidget_plugins->currentIndex().parent().isValid();
    ui.widget_pluginInfos->setEnabled(!isTopLevelItem);
    if (isTopLevelItem) return;

    QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
    if (_pluginManager->plugins().contains(path)){
        PluginLoader* plugin = _pluginManager->plugins()[path];
        ui.label_pluginName->setText(plugin->name());
        ui.label_pluginVersion->setText(plugin->version());
        ui.label_pluginCopyright->setText(plugin->copyright());
        ui.label_pluginGroup->setText(plugin->group());
        ui.label_pluginPath->setText(ui.label_pluginPath->fontMetrics().elidedText(plugin->fileName(), Qt::ElideMiddle,ui.label_pluginPath->width()));
        ui.label_pluginPath->setToolTip(plugin->fileName());
        ui.label_pluginPlatform->setText(plugin->platform());
        QString deps;
        for (QString s : plugin->dependencies())
            deps.append(s).append("\n");
        ui.textBrowser_pluginDependencies->setText(deps);
        ui.textBrowser_pluginDescription->setText(plugin->description());
        ui.pushButton_pluginHelp->setEnabled(plugin->status() == PluginLoader::Status::Loaded);
        ui.pushButton_pluginConfig->setEnabled(plugin->status() == PluginLoader::Status::Loaded);
    }
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
    }
    else
    {
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
    if (!_mainWidget->setTheme(ui.comboBox_themes->itemText(i))){
        QMessageBox msgBox(QMessageBox::Critical, "Error", "Could not apply theme.");
        msgBox.exec();
        if (!_mainWidget->setTheme(currentTheme)){
           qFatal("Rolling back theme failed.");
        }
    }
}



/** ***************************************************************************/
void SettingsWidget::show() {
    QWidget::show();
    this->move(QApplication::desktop()->screenGeometry().center() - rect().center());
}



/******************************************************************************/
/*                            O V E R R I D E S                               */
/******************************************************************************/



/** ***************************************************************************/
void SettingsWidget::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape ) {
        close();
    }
}



/** ***************************************************************************/
void SettingsWidget::closeEvent(QCloseEvent *event) {
    if (_hotkeyManager->hotkeys().empty()){
        QMessageBox msgBox(QMessageBox::Critical, "Error",
                           "Hotkey is invalid, please set it. Press Ok to go"\
                           "back to the settings, or press Cancel to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ){
            ui.tabs->setCurrentIndex(0);
            this->show();
        }
        else
            qApp->quit();
        event->ignore();
        return;
    }
    event->accept();
}
