// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#include "settingsdialog.h"

#include "globalhotkey.h"
#include "mainwidget.h"

#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFocusEvent>
#include "pluginhandler.h"
#include "settings.h"



/****************************************************************************///
SettingsWidget::SettingsWidget(QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent,f)
{
    qDebug() << "Call to SettingsWidget::ctor";
	ui.setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
//    setWindowModality(Qt::ApplicationModal);


    // Always center
    ui.checkBox_center->setChecked(gSettings->value(CFG_CENTERED, true).toBool());
    connect(ui.checkBox_center, &QCheckBox::clicked,
            [](bool b){ gSettings->setValue(CFG_CENTERED, b); });

//    // Max History
//    ui.sb_maxHistory->setValue(_mainWidget->_inputLine->_history._max);
//    connect(ui.sb_maxHistory, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
//            this, &SettingsWidget::onMaxHistoryChanged);


    // Plugin tab
    updatePluginList();
    connect(ui.treeWidget_plugins, &QTreeWidget::currentItemChanged,
            this, &SettingsWidget::updatePluginInformations);


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






//	// Proposal stuff
//	ui.spinBox_proposals->setValue(gSettings->value(CFG_ITEMCOUNT, CFG_DEF_ITEMCOUNT));
//	connect(ui.spinBox_proposals, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
//			this, &SettingsWidget::onNItemsChanged);

//	// Load subtext mode for selected items
//	ui.cb_subModeSel->setCurrentIndex((int)_mainWidget->_proposalListView->_selSubtextMode);
//	// Apply changes made to subtext mode of selected items
//	connect(ui.cb_subModeSel, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
//			this, &SettingsWidget::onSubModeSelChanged);

//	// Load subtext mode for unselected items
//	ui.cb_subModeDef->setCurrentIndex((int)_mainWidget->_proposalListView->_defSubtextMode);
//	// Apply changes made to subtext mode of "UN"selected items
//	connect(ui.cb_subModeDef, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
//			this, &SettingsWidget::onSubModeDefChanged);



	/*
	 * STYLE
	 */

//	// Get all themes and add them to the cb
//	QStringList themeDirs =
//			QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes",
//									  QStandardPaths::LocateDirectory);
//	QFileInfoList themes;
//	for (QDir d : themeDirs)
//		themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
//	int i = 0 ;
//	for (QFileInfo fi : themes){
//		ui.cb_themes->addItem(fi.baseName(), fi.canonicalFilePath());
//		if ( fi.baseName() == _mainWidget->_theme)
//			ui.cb_themes->setCurrentIndex(i);
//		++i;
//	}

//	// Apply a skin if clicked
//	connect(ui.cb_themes, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
//			this, &SettingsWidget::onThemeChanged);



//	/*
//	 * ACTION MODIFIERS
//	 */
//	ui.cb_modActionCtrl->setCurrentIndex(_mainWidget->_proposalListView->_actionCtrl);
//	connect(ui.cb_modActionCtrl, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
//			this, &SettingsWidget::modActionCtrlChanged);
//	ui.cb_modActionMeta->setCurrentIndex(_mainWidget->_proposalListView->_actionMeta);
//	connect(ui.cb_modActionMeta, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
//			this, &SettingsWidget::modActionMetaChanged);
//	ui.cb_modActionAlt->setCurrentIndex(_mainWidget->_proposalListView->_actionAlt);
//	connect(ui.cb_modActionAlt, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
//			this, &SettingsWidget::modActionAltChanged);



//	/*
//	 *  MODULES
//	 */
//	for (Service* m : _mainWidget->_engine->_modules){
//		QListWidgetItem *item = new QListWidgetItem(m->moduleName());
//		ui.lw_modules->addItem(item);
//		ui.sw_modules->addWidget(m->widget());
//	}




//	/* GENERAL APPEARANCE OF SETTINGSWISGET */
//	// Set the width of the list to the with of the content
//	ui.lw_modules->setFixedWidth(ui.lw_modules->sizeHintForColumn(0)
//								 + ui.lw_modules->contentsMargins().left()
//								 + ui.lw_modules->contentsMargins().right()
//								 + ui.lw_modules->spacing()*2);
    //	ui.lw_modules->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/****************************************************************************///
SettingsWidget::~SettingsWidget()
{

    qDebug() << "Call to SettingsWidget::dtor";
}

/****************************************************************************///
void SettingsWidget::updatePluginList()
{
    QSet<QString> blacklist = gSettings->value(CFG_PLGN_BLACKLIST).toStringList().toSet();
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
//        child->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        child->setData(0, Qt::DisplayRole, spec.name);
        child->setData(1, Qt::DisplayRole, spec.version);
            child->setCheckState(0, (blacklist.contains(spec.name)) ? Qt::Unchecked : Qt::Checked);
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
            ui.label_pluginPath->setText(spec.path);
            ui.label_pluginPlatform->setText(spec.platform);
            QString deps;
            for (QString s : spec.dependencies)
                deps.append(s).append("\n");
            ui.textBrowser_pluginDependencies->setText(deps);
            ui.textBrowser_pluginDescription->setText(spec.description);
            break;
        }
    }
}



/******************************************************************************/
/**************************** O V E R R I D E S *******************************/
/******************************************************************************/


/****************************************************************************///
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
}


/******************************************************************************/
/*******************************  S L O T S  **********************************/
/******************************************************************************/


/****************************************************************************///
void SettingsWidget::changeHotkey(int newhk)
{
    int oldhk = *gHotkeyManager->hotkeys().begin(); //OMG

    // Cancel
    if (newhk == Qt::Key_Escape){
        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
        gHotkeyManager->enable();
        return;
    }

    // Try to set the hotkey
    if (gHotkeyManager->registerHotkey(newhk)) {
        ui.grabKeyButton_hotkey->setText(QKeySequence(newhk).toString());
        gSettings->setValue(CFG_HOTKEY, QKeySequence(newhk).toString());
        gHotkeyManager->unregisterHotkey(oldhk);
    }
    else
    {
        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
        QMessageBox(QMessageBox::Critical, "Error",
                    QKeySequence(newhk).toString()
                    + " could not be registered.").exec();
    }
    gHotkeyManager->enable();
}


/****************************************************************************///
void SettingsWidget::onThemeChanged(int i)
{
//	// Apply and save the theme
//	QFile themeFile(ui.cb_themes->itemData(i).toString());
//	if (themeFile.open(QFile::ReadOnly)) {
//		qApp->setStyleSheet(themeFile.readAll());
//		_mainWidget->_theme = ui.cb_themes->itemText(i);
//		themeFile.close();
//		return;
//	} else {
//		QMessageBox msgBox(QMessageBox::Critical, "Error", "Could not open theme file.");
//		msgBox.exec();
//	}
}

///****************************************************************************///
//void SettingsWidget::onNItemsChanged(int i)
//{
//	_mainWidget->_proposalListView->_nItemsToShow = i;
//	_mainWidget->_proposalListView->updateGeometry();
//}

///****************************************************************************///
//void SettingsWidget::onSubModeSelChanged(int option)
//{
//	_mainWidget->_proposalListView->setSubModeSel(static_cast<ProposalListView::SubTextMode>(option));
//}

///****************************************************************************///
//void SettingsWidget::onSubModeDefChanged(int option)
//{
//	_mainWidget->_proposalListView->setSubModeDef(static_cast<ProposalListView::SubTextMode>(option));
//}

///****************************************************************************///
//void SettingsWidget::modActionCtrlChanged(int i)
//{
//	_mainWidget->_proposalListView->_actionCtrl = i;
//}

///****************************************************************************///
//void SettingsWidget::modActionMetaChanged(int i)
//{
//	_mainWidget->_proposalListView->_actionMeta = i;
//}

///****************************************************************************///
//void SettingsWidget::modActionAltChanged(int i)
//{
//	_mainWidget->_proposalListView->_actionAlt = i;
//}

/****************************************************************************///
void SettingsWidget::show()
{
	QWidget::show();
	this->move(QApplication::desktop()->screenGeometry().center() - rect().center());
}
