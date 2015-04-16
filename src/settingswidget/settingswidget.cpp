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
#include "globalhotkey.h"
#include "mainwidget.h"
#include "pluginhandler.h"
#include "settings.h"
#include "plugininterfaces/extensioninterface.h"

/****************************************************************************///
SettingsWidget::SettingsWidget(QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent,f)
{
	ui.setupUi(this);
    setWindowFlags(Qt::Window|Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);


    /*
     * GENERAL TAB
     */

    // HOTKEY STUFF
   QSet<int> hks = gHotkeyManager->hotkeys();
   if (hks.size() < 1)
       ui.grabKeyButton_hotkey->setText("Press to set hotkey");
   else
       ui.grabKeyButton_hotkey->setText(QKeySequence(*hks.begin()).toString()); // OMG
   connect(ui.grabKeyButton_hotkey, &GrabKeyButton::clicked,
           gHotkeyManager, &GlobalHotkey::disable);
   connect(ui.grabKeyButton_hotkey, &GrabKeyButton::keyCombinationPressed,
           this, &SettingsWidget::changeHotkey);


   // MAX PROPOSALS
   ui.spinBox_proposals->setValue(gSettings->value(CFG_MAX_PROPOSALS, CFG_MAX_PROPOSALS_DEF).toInt());
   connect(ui.spinBox_proposals, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
           [](int i){ gSettings->setValue(CFG_MAX_PROPOSALS, i); });


    // ALWAYS CENTER
    ui.checkBox_center->setChecked(gSettings->value(CFG_CENTERED, CFG_CENTERED_DEF).toBool());
    connect(ui.checkBox_center, &QCheckBox::toggled,
            [](bool b){ gSettings->setValue(CFG_CENTERED, b); });


    // SUBTEXT SELECTED
    ui.checkBox_showInfo->setChecked(gSettings->value(CFG_SHOW_INFO, CFG_SHOW_INFO_DEF).toBool());
    connect(ui.checkBox_showInfo, &QCheckBox::toggled,
            [](bool b){ gSettings->setValue(CFG_SHOW_INFO, b); });


    // SUBTEXT UNSELECTED
    ui.checkBox_showAction->setChecked(gSettings->value(CFG_SHOW_ACTION, CFG_SHOW_ACTION_DEF).toBool());
    connect(ui.checkBox_showAction, &QCheckBox::toggled,
            [](bool b){ gSettings->setValue(CFG_SHOW_ACTION, b); });

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
        if ( fi.baseName() == gSettings->value(CFG_THEME, CFG_THEME_DEF).toString())
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
    connect(ui.treeWidget_plugins, &QTreeWidget::itemChanged,
            [=](QTreeWidgetItem * item, int column){
        QSet<QString> blackSet =gSettings->value(CFG_BLACKLIST).toStringList().toSet();
        if ( static_cast<bool>(item->checkState(column)) )
            blackSet.remove(item->text(0));
        else
            blackSet.insert(item->text(0));
        gSettings->setValue(CFG_BLACKLIST, QVariant(QStringList::fromSet(blackSet)));
    });

    connect(ui.pushButton_pluginHelp, &QPushButton::clicked,
            this, &SettingsWidget::openPluginHelp);

    connect(ui.pushButton_pluginConfig, &QPushButton::clicked,
            this, &SettingsWidget::openPluginConfig);
}

/****************************************************************************///
SettingsWidget::~SettingsWidget()
{
}

/** ***************************************************************************/
void SettingsWidget::openPluginHelp()
{
    if (ui.treeWidget_plugins->currentIndex().isValid()){
        QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
        for (const PluginSpec& spec : PluginHandler::instance()->getPluginSpecs()){
            if (spec.path == path) { // MUST HAPPEN
                QWidget *w = dynamic_cast<AbstractPluginInterface*>(spec.loader->instance())->widget();
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
void SettingsWidget::openPluginConfig()
{
    if (ui.treeWidget_plugins->currentIndex().isValid()){
        QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
        for (const PluginSpec& spec : PluginHandler::instance()->getPluginSpecs()){
            if (spec.path == path) { // MUST HAPPEN
                QWidget *w = dynamic_cast<AbstractPluginInterface*>(spec.loader->instance())->widget();
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

/****************************************************************************///
void SettingsWidget::updatePluginList()
{
    const QList<PluginSpec>& specs = PluginHandler::instance()->getPluginSpecs();
    for (const PluginSpec & spec : specs){

        // Get the top level item of this group, make sure it exists
        QList<QTreeWidgetItem *> tlis =
                ui.treeWidget_plugins->findItems(spec.group, Qt::MatchExactly);
        if (tlis.size() > 1) // MUST NOT HAPPEN
            qCritical() << "Found multiple identical TopLevelItems in PluginList";
        QTreeWidgetItem *tli;
        if (tlis.size() == 0){
            tli= new QTreeWidgetItem({spec.group});
            tli->setFlags(Qt::ItemIsEnabled);
            ui.treeWidget_plugins->addTopLevelItem(tli);
        }
        else
            tli = tlis.first();

        // Create the child and insert a childitem
        QTreeWidgetItem *child = new QTreeWidgetItem();
        child->setChildIndicatorPolicy(QTreeWidgetItem::QTreeWidgetItem::DontShowIndicatorWhenChildless);
        child->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        child->setData(0, Qt::DisplayRole, spec.name);
        child->setData(0, Qt::ToolTipRole, spec.description);
        child->setData(1, Qt::ToolTipRole, "Load at boot");
        if (gSettings->value(CFG_BLACKLIST).toStringList().contains(spec.name ))
            child->setCheckState(1, Qt::Unchecked);
        else
            child->setCheckState(1, Qt::Checked);
        switch (spec.status) {
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
        child->setData(0, Qt::UserRole, spec.path);
        child->setData(0, Qt::ToolTipRole, spec.description);
        tli->addChild(child);
    }
}

/****************************************************************************///
void SettingsWidget::updatePluginInformations()
{
    // Respect the toplevel items
    bool isTopLevelItem = !ui.treeWidget_plugins->currentIndex().parent().isValid();
    ui.widget_pluginInfos->setEnabled(!isTopLevelItem);
    if (isTopLevelItem) return;

    //    FIND AND APPLY
    QString path = ui.treeWidget_plugins->currentItem()->data(0,Qt::UserRole).toString();
    for (const PluginSpec& spec : PluginHandler::instance()->getPluginSpecs()){
        if (spec.path == path) { // MUST HAPPEN
            ui.label_pluginName->setText(spec.name);
            ui.label_pluginVersion->setText(spec.version);
            ui.label_pluginCopyright->setText(spec.copyright);
            ui.label_pluginGroup->setText(spec.group);
            ui.label_pluginPath->setText(ui.label_pluginPath->fontMetrics().elidedText(spec.path, Qt::ElideMiddle,ui.label_pluginPath->width()));
            ui.label_pluginPath->setToolTip(spec.path);
            ui.label_pluginPlatform->setText(spec.platform);
            QString deps;
            for (QString s : spec.dependencies)
                deps.append(s).append("\n");
            ui.textBrowser_pluginDependencies->setText(deps);
            ui.textBrowser_pluginDescription->setText(spec.description);
            ui.pushButton_pluginHelp->setEnabled(spec.status == PluginSpec::Status::Loaded);
            ui.pushButton_pluginConfig->setEnabled(spec.status == PluginSpec::Status::Loaded);
            break;
        }
    }
}

/****************************************************************************///
void SettingsWidget::changeHotkey(int newhk)
{
    int oldhk = *gHotkeyManager->hotkeys().begin(); //TODO Make cool sharesdpointer design

    // Try to set the hotkey
    if (gHotkeyManager->registerHotkey(newhk)) {
        QString hkText(QKeySequence((newhk&~Qt::GroupSwitchModifier)).toString());//QTBUG-45568
        ui.grabKeyButton_hotkey->setText(hkText);
        gSettings->setValue(CFG_HOTKEY, hkText);
        gHotkeyManager->unregisterHotkey(oldhk);
    }
    else
    {
        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
        QMessageBox(QMessageBox::Critical, "Error",
                    QKeySequence(newhk).toString()
                    + " could not be registered.").exec();
    }
}

/****************************************************************************///
void SettingsWidget::onThemeChanged(int i)
{
    // Apply and save the theme
    QFile themeFile(ui.comboBox_themes->itemData(i).toString());
    if (themeFile.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(themeFile.readAll());
        gSettings->setValue(CFG_THEME, ui.comboBox_themes->itemText(i));
        themeFile.close();
        return;
    } else {
        QMessageBox msgBox(QMessageBox::Critical, "Error", "Could not open theme file.");
        msgBox.exec();
    }
}

/** ***************************************************************************/
void SettingsWidget::show()
{
    QWidget::show();
    this->move(QApplication::desktop()->screenGeometry().center() - rect().center());
}

/******************************************************************************/
/*                            O V E R R I D E S                               */
/******************************************************************************/

/** ***************************************************************************/
void SettingsWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape ) {
        close();
    }
}

/** ***************************************************************************/
void SettingsWidget::closeEvent(QCloseEvent *event)
{
    if (gHotkeyManager->hotkeys().empty()){
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
