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

#include "services/websearch/websearch.h"
#include "services/fileindex/fileindex.h"
#include "services/calculator/calculator.h"
#include "services/bookmarkindex/bookmarkindex.h"
#include "services/appindex/appindex.h"
#include "globalhotkey.h"
#include "mainwidget.h"

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFocusEvent>
#include <QSettings>


/**************************************************************************/
SettingsWidget::SettingsWidget(MainWidget *ref)
	: _mainWidget(ref)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window);



	/* HOTKEY STUFF */
	_waitingForHotkey = false;
	const int hk = _mainWidget->_hotkeyManager.hotkey();
	ui.pb_hotkey->setText((hk==0)
				  ? "Press to set hotkey"
				  : QKeySequence(hk).toString());
	connect(ui.pb_hotkey, &QPushButton::clicked,
			this, &SettingsWidget::onPbHotkeyPressed);



	// Always center
	ui.cb_center->setChecked(_mainWidget->_showCentered);
	connect(ui.cb_center, &QCheckBox::clicked,
			this, &SettingsWidget::onShowCenteredChanged);


	// Max History
	ui.sb_maxHistory->setValue(_mainWidget->_inputLine->_history._max);
	connect(ui.sb_maxHistory, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
			this, &SettingsWidget::onMaxHistoryChanged);


	/* GENERAL */
	// Proposal stuff
	ui.spinBox_proposals->setValue(_mainWidget->_proposalListView->_nItemsToShow);

	// Apply changes made to the amount of proposals
	connect(ui.spinBox_proposals, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
			this, &SettingsWidget::onNItemsChanged);

	// Load subtext mode for selected items
	ui.cb_subModeSel->setCurrentIndex((int)_mainWidget->_proposalListView->_selSubtextMode);
	// Apply changes made to subtext mode of selected items
	connect(ui.cb_subModeSel, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
			this, &SettingsWidget::onSubModeSelChanged);

	// Load subtext mode for unselected items
	ui.cb_subModeDef->setCurrentIndex((int)_mainWidget->_proposalListView->_defSubtextMode);
	// Apply changes made to subtext mode of "UN"selected items
	connect(ui.cb_subModeDef, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
			this, &SettingsWidget::onSubModeDefChanged);



	/* STYLE */
	// Get all themes and add them to the cb
	QStringList themeDirs =
			QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes",
									  QStandardPaths::LocateDirectory);
	QFileInfoList themes;
	for (QDir d : themeDirs)
		themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
	int i = 0 ;
	for (QFileInfo fi : themes){
		ui.cb_themes->addItem(fi.baseName(), fi.canonicalFilePath());
		if ( fi.baseName() == _mainWidget->_theme)
			ui.cb_themes->setCurrentIndex(i);
		++i;
	}

	// Apply a skin if clicked
	connect(ui.cb_themes, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
			this, &SettingsWidget::onThemeChanged);



	/* ACTION MODIFIERS */
	ui.cb_modActionCtrl->setCurrentIndex(_mainWidget->_proposalListView->_actionCtrl);
	connect(ui.cb_modActionCtrl, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
			this, &SettingsWidget::modActionCtrlChanged);
	ui.cb_modActionMeta->setCurrentIndex(_mainWidget->_proposalListView->_actionMeta);
	connect(ui.cb_modActionMeta, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
			this, &SettingsWidget::modActionMetaChanged);
	ui.cb_modActionAlt->setCurrentIndex(_mainWidget->_proposalListView->_actionAlt);
	connect(ui.cb_modActionAlt, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
			this, &SettingsWidget::modActionAltChanged);



	/* MODULES */
	for (Service* m : _mainWidget->_engine->_modules){
		QListWidgetItem *item = new QListWidgetItem(m->moduleName());
		ui.lw_modules->addItem(item);
		ui.sw_modules->addWidget(m->widget());
	}




	/* GENERAL APPEARANCE OF SETTINGSWISGET */
	// Set the width of the list to the with of the content
	ui.lw_modules->setFixedWidth(ui.lw_modules->sizeHintForColumn(0)
								 + ui.lw_modules->contentsMargins().left()
								 + ui.lw_modules->contentsMargins().right()
								 + ui.lw_modules->spacing()*2);
	ui.lw_modules->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/**************************************************************************/
void SettingsWidget::grabAll()
{
	grabKeyboard();
	grabMouse();
	_waitingForHotkey = true;
}

/**************************************************************************/
void SettingsWidget::releaseAll()
{
	releaseKeyboard();
	releaseMouse();
	_waitingForHotkey = false;
}


/******************************************************************************/
/**************************** O V E R R I D E S *******************************/
/******************************************************************************/


/**************************************************************************/
void SettingsWidget::closeEvent(QCloseEvent *event)
{
	if (_mainWidget->_hotkeyManager.hotkey() == 0){
		QMessageBox msgBox(QMessageBox::Critical, "Error",
						   "Hotkey is invalid, please set it. Press Ok to go"\
						   "back to the settings, or press Cancel to quit albert.",
						   QMessageBox::Close|QMessageBox::Ok);
		msgBox.exec();
		if ( msgBox.result() == QMessageBox::Ok ){
			this->show(SettingsWidget::Tab::General);
			ui.tabs->setCurrentIndex(0);
		}
		else
			qApp->quit();
		event->ignore();
		return;
	}
}

/**************************************************************************/
void SettingsWidget::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();
	int mods = event->modifiers();

	if ( _waitingForHotkey )
	{
		// Modifier pressed -> update the label
		if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
			ui.pb_hotkey->setText(QKeySequence(mods|Qt::Key_Question).toString());
			return;
		}

		// Cancel
		if (key == Qt::Key_Escape){
			ui.pb_hotkey->setText("Press to set hotkey");
			releaseAll();
			return;
		}

		// Try to register a hotkey
		releaseAll();
		if (!_mainWidget->_hotkeyManager.registerHotkey(mods|key) )
		{
			QMessageBox msgBox(QMessageBox::Critical, "Error",
							   QKeySequence(mods|key).toString()
							   + " could not be registered.");
			msgBox.exec();
			ui.pb_hotkey->setText("Press to set hotkey");
		}
		else
			ui.pb_hotkey->setText(QKeySequence(mods|key).toString());

		return;
	}

	if (key == Qt::Key_Escape)
		close();

	QWidget::keyPressEvent( event );
}


/**************************************************************************/
void SettingsWidget::keyReleaseEvent(QKeyEvent *event)
{
	if ( _waitingForHotkey ) {
		// Modifier released -> update the label
		int key = event->key();
		if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
			ui.pb_hotkey->setText(QKeySequence(event->modifiers()|Qt::Key_Question).toString());
			return;
		}
		return;
	}
	QWidget::keyReleaseEvent( event );
}


/******************************************************************************/
/*******************************  S L O T S  **********************************/
/******************************************************************************/


/**************************************************************************/
void SettingsWidget::onHotkeyChanged(int hotkey)
{
	ui.pb_hotkey->setText(QKeySequence(hotkey).toString());
}

/**************************************************************************/
void SettingsWidget::onShowCenteredChanged(bool b)
{
	_mainWidget->_showCentered = b;
}

/**************************************************************************/
void SettingsWidget::onMaxHistoryChanged(int i)
{
	_mainWidget->_inputLine->_history._max = i;
}

/**************************************************************************/
void SettingsWidget::onThemeChanged(int i)
{
	// Apply and save the theme
	QFile themeFile(ui.cb_themes->itemData(i).toString());
	if (themeFile.open(QFile::ReadOnly)) {
		qApp->setStyleSheet(themeFile.readAll());
		_mainWidget->_theme = ui.cb_themes->itemText(i);
		themeFile.close();
		return;
	} else {
		QMessageBox msgBox(QMessageBox::Critical, "Error", "Could not open theme file.");
		msgBox.exec();
	}
}

/**************************************************************************/
void SettingsWidget::onNItemsChanged(int i)
{
	_mainWidget->_proposalListView->_nItemsToShow = i;
	_mainWidget->_proposalListView->updateGeometry();
}

/**************************************************************************/
void SettingsWidget::onSubModeSelChanged(int option)
{
	_mainWidget->_proposalListView->setSubModeSel(static_cast<ProposalListView::SubTextMode>(option));
}

/**************************************************************************/
void SettingsWidget::onSubModeDefChanged(int option)
{
	_mainWidget->_proposalListView->setSubModeDef(static_cast<ProposalListView::SubTextMode>(option));
}

/**************************************************************************/
void SettingsWidget::modActionCtrlChanged(int i)
{
	_mainWidget->_proposalListView->_actionCtrl = i;
}

/**************************************************************************/
void SettingsWidget::modActionMetaChanged(int i)
{
	_mainWidget->_proposalListView->_actionMeta = i;
}

/**************************************************************************/
void SettingsWidget::modActionAltChanged(int i)
{
	_mainWidget->_proposalListView->_actionAlt = i;
}

/**************************************************************************/
void SettingsWidget::onPbHotkeyPressed()
{
	_mainWidget->_hotkeyManager.unregisterHotkey();
	ui.pb_hotkey->setText("?");
	grabAll();
}

/**************************************************************************/
void SettingsWidget::show()
{
	QWidget::show();
	this->move(QApplication::desktop()->screenGeometry().center() - rect().center());
}

/**************************************************************************/
void SettingsWidget::show(SettingsWidget::Tab t)
{
	show();
	ui.tabs->setCurrentIndex(static_cast<int>(t));
}
