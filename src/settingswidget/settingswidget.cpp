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

#include "settingswidget.h"
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShortcut>
#include <QDesktopWidget>
#include <QFocusEvent>
#include "hotkeymanager.h"
#include "mainwidget.h"
#include "pluginhandler.h"
#include "plugininterfaces/extension_if.h"


/** ***************************************************************************/
SettingsWidget::SettingsWidget(MainWidget *mainWidget, HotkeyManager *hotkeyManager, PluginHandler *pluginHandler, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), _mainWidget(mainWidget), _hotkeyManager(hotkeyManager), _pluginHandler(pluginHandler) {

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
    // SUBTEXT UNSELECTED
    ui.checkBox_showAction->setChecked(mainWidget->ui.proposalList->showAction());
    connect(ui.checkBox_showAction, &QCheckBox::toggled, mainWidget->ui.proposalList, &ProposalList::setShowAction);


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
    if (ui.treeWidget_plugins->currentIndex().isValid()){
        QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
        for (PluginSpec *spec : _pluginHandler->pluginSpecs()){
            if (spec->path == path) { // MUST HAPPEN
                QWidget *w = dynamic_cast<PluginInterface*>(spec->loader->instance())->widget();
                w->setParent(this);
                w->setWindowTitle("Plugin help");
                w->setWindowFlags(Qt::Window|Qt::WindowCloseButtonHint);
                w->setAttribute(Qt::WA_DeleteOnClose);
                new QShortcut(Qt::Key_Escape, w, SLOT(close()));
                w->move(w->parentWidget()->window()->frameGeometry().topLeft() +
                        w->parentWidget()->window()->rect().center() -
                        w->rect().center());
                w->show();
            }
        }
    }
}



/** ***************************************************************************/
void SettingsWidget::openPluginConfig() {
    if (ui.treeWidget_plugins->currentIndex().isValid()){
        QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
        for (const PluginSpec* spec : _pluginHandler->pluginSpecs()){
            if (spec->path == path) { // MUST HAPPEN
                QWidget *w = dynamic_cast<PluginInterface*>(spec->loader->instance())->widget();
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
}



/** ***************************************************************************/
void SettingsWidget::onPluginItemChanged(QTreeWidgetItem *item, int column) {
    if ( static_cast<bool>(item->checkState(column)) )
        _pluginHandler->blacklist().removeAll(item->text(0));
    else
        _pluginHandler->blacklist().append(item->text(0));
    ui.label_restart->setVisible(true);
}



/** ***************************************************************************/
void SettingsWidget::updatePluginList() {
    const QList<PluginSpec*>& specs = _pluginHandler->pluginSpecs();
    for (const PluginSpec* spec : specs){

        // Get the top level item of this group, make sure it exists
        QList<QTreeWidgetItem *> tlis =
                ui.treeWidget_plugins->findItems(spec->group, Qt::MatchExactly);
        if (tlis.size() > 1) // MUST NOT HAPPEN
            qCritical() << "Found multiple identical TopLevelItems in PluginList";
        QTreeWidgetItem *tli;
        if (tlis.size() == 0){
            tli= new QTreeWidgetItem({spec->group});
            tli->setFlags(Qt::ItemIsEnabled);
            ui.treeWidget_plugins->addTopLevelItem(tli);
        }
        else
            tli = tlis.first();

        // Create the child and insert a childitem
        QTreeWidgetItem *child = new QTreeWidgetItem();
        child->setChildIndicatorPolicy(QTreeWidgetItem::QTreeWidgetItem::DontShowIndicatorWhenChildless);
        child->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        child->setData(0, Qt::DisplayRole, spec->name);
        child->setData(0, Qt::ToolTipRole, spec->description);
        child->setData(1, Qt::ToolTipRole, "Load at boot");
        if (_pluginHandler->blacklist().contains(spec->name ))
            child->setCheckState(1, Qt::Unchecked);
        else
            child->setCheckState(1, Qt::Checked);
        switch (spec->status) {
        case PluginSpec::Status::Loaded:
            child->setData(0, Qt::DecorationRole, QIcon(":plugin_loaded"));
            break;
        case PluginSpec::Status::NotLoaded:
            child->setData(0, Qt::DecorationRole, QIcon(":plugin_notloaded"));
            break;
        case PluginSpec::Status::Error:
            child->setData(0, Qt::DecorationRole, QIcon(":plugin_error"));
            break;
        default:
            qCritical() << "Unhandled PluginSpec::Status";
        }
        child->setData(0, Qt::UserRole, spec->path);
        child->setData(0, Qt::ToolTipRole, spec->description);
        tli->addChild(child);
    }
}



/** ***************************************************************************/
void SettingsWidget::updatePluginInformations() {
    // Respect the toplevel items
    bool isTopLevelItem = !ui.treeWidget_plugins->currentIndex().parent().isValid();
    ui.widget_pluginInfos->setEnabled(!isTopLevelItem);
    if (isTopLevelItem) return;

    //    FIND AND APPLY
    QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
    for (const PluginSpec* spec : _pluginHandler->pluginSpecs()){
        if (spec->path == path) { // MUST HAPPEN
            ui.label_pluginName->setText(spec->name);
            ui.label_pluginVersion->setText(spec->version);
            ui.label_pluginCopyright->setText(spec->copyright);
            ui.label_pluginGroup->setText(spec->group);
            ui.label_pluginPath->setText(ui.label_pluginPath->fontMetrics().elidedText(spec->path, Qt::ElideMiddle,ui.label_pluginPath->width()));
            ui.label_pluginPath->setToolTip(spec->path);
            ui.label_pluginPlatform->setText(spec->platform);
            QString deps;
            for (QString s : spec->dependencies)
                deps.append(s).append("\n");
            ui.textBrowser_pluginDependencies->setText(deps);
            ui.textBrowser_pluginDescription->setText(spec->description);
            ui.pushButton_pluginHelp->setEnabled(spec->status == PluginSpec::Status::Loaded);
            ui.pushButton_pluginConfig->setEnabled(spec->status == PluginSpec::Status::Loaded);
            break;
        }
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
