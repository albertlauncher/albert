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
#include "globals.h"
#include "globalhotkey.h"
#include "mainwidget.h"

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>


/**************************************************************************/
SettingsWidget::SettingsWidget(MainWidget *ref)
	: _mainWidget(ref)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window);
//	setAttribute(Qt::WA_DeleteOnClose);


	/* GENERAL */

	// Proposal stuff
	ui.spinBox_proposals->setValue(gSettings->value("nItemsToShow", 5).toInt());

	// Apply changes made to the amount of proposals
	connect(ui.spinBox_proposals, SIGNAL(valueChanged(int)), this, SLOT(onNItemsChanged(int)));

	// Load subtext mode for selected items
	ui.cb_subModeSel->setCurrentIndex(gSettings->value("subModeSelected", 2).toInt());
	// Apply changes made to subtext mode of selected items
	connect(ui.cb_subModeSel, SIGNAL(currentIndexChanged(int)),
			this, SLOT(onSubModeSelChanged(int)));

	// Load subtext mode for unselected items
	ui.cb_subModeDef->setCurrentIndex(gSettings->value("subModeDefault",  1).toInt());
	// Apply changes made to subtext mode of "UN"selected items
	connect(ui.cb_subModeDef, SIGNAL(currentIndexChanged(int)),
			this, SLOT(onSubModeDefChanged(int)));


	// Appearance // add a fallback
	ui.cb_themes->addItem("Standard (Fallback)");
	QStringList themeDirs = QStandardPaths::locateAll(QStandardPaths::DataLocation,
													  "themes",
													  QStandardPaths::LocateDirectory);
	// Get all themes and add them
	QFileInfoList themes;
	for (QDir d : themeDirs)
		themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);
	int i = 1;
	for (QFileInfo fi : themes){
		ui.cb_themes->addItem(fi.baseName(), fi.canonicalFilePath());
		if ( fi.baseName() == gSettings->value("theme").toString() )
			ui.cb_themes->setCurrentIndex(i);
		++i;
	}

	// Apply a skin if clicked
	connect(ui.cb_themes, SIGNAL(currentIndexChanged(int)),
			this, SLOT(onThemeChanged(int)));


	/* MODULES */

	QListWidgetItem *item = new QListWidgetItem("Websearch"); // TODO NO SINGLETONS!!!
	ui.lw_modules->addItem(item);
	ui.sw_modules->addWidget(WebSearch::instance()->widget());

	item = new QListWidgetItem("Apps");
	ui.lw_modules->addItem(item);
	ui.sw_modules->addWidget(AppIndex::instance()->widget());

	item = new QListWidgetItem("Bookmarks");
	ui.lw_modules->addItem(item);
	ui.sw_modules->addWidget(BookmarkIndex::instance()->widget());

	item = new QListWidgetItem("Files");
	ui.lw_modules->addItem(item);
	ui.sw_modules->addWidget(FileIndex::instance()->widget());

	item = new QListWidgetItem("Calculator");
	ui.lw_modules->addItem(item);
	ui.sw_modules->addWidget(Calculator::instance()->widget());

	// The entries and the stackeswidgets are connected in the .ui

	// Set the width of the list to the with of the content
	ui.lw_modules->setFixedWidth(ui.lw_modules->sizeHintForColumn(0)
								 + ui.lw_modules->contentsMargins().left()
								 + ui.lw_modules->contentsMargins().right()
								 + ui.lw_modules->spacing()*2);
}

/**************************************************************************/
void SettingsWidget::closeEvent(QCloseEvent *event)
{
	if (GlobalHotkey::instance()->hotkey() == 0){
		QMessageBox msgBox(QMessageBox::Critical, "Error",
						   "Hotkey is not set or invalid. Please set it in the settings. "\
						   "Press Ok to go back to the settings, or press Cancel to quit albert.",
						   QMessageBox::Close|QMessageBox::Ok);
		msgBox.exec();
		if ( msgBox.result() == QMessageBox::Ok ){
			this->show(SettingsWidget::Tab::General);
			ui.tabs->setCurrentIndex(0);
		}
		else
			qApp->quit();
		event->ignore();
	}
}

/******************************************************************************/
/*******************************  S L O T S  **********************************/
/******************************************************************************/


/**************************************************************************/
void SettingsWidget::onThemeChanged(int i)
{
	QString path(ui.cb_themes->itemData(i).toString());

	// Special case if variant is empty use fallback resource
	if (path.isEmpty()) {
		qApp->setStyleSheet(QString::fromLocal8Bit("file:///:/resources/Standard.qss"));
		gSettings->remove("theme");
		return;
	}

	// Apply and save the theme
	QFileInfo fi(path);
	if (fi.exists()) {
		QFile styleFile(fi.canonicalFilePath());
		if (styleFile.open(QFile::ReadOnly)) {
			qApp->setStyleSheet(styleFile.readAll());
			gSettings->setValue("theme", fi.baseName());
			styleFile.close();
			return;
		}
	}

	// Error remove setting an give feedback
	qApp->setStyleSheet(QString::fromLocal8Bit("file:///:/resources/Standard.qss"));
	gSettings->remove("theme");
	QMessageBox msgBox(QMessageBox::Critical, "Error", "Could not open theme file.");
	msgBox.exec();
}

/**************************************************************************/
void SettingsWidget::onNItemsChanged(int i)
{
	gSettings->setValue("nItemsToShow", i);
	_mainWidget->_proposalListView->updateGeometry();
}

/**************************************************************************/
void SettingsWidget::onSubModeSelChanged(int option)
{
	gSettings->setValue("subModeSelected", option);
	_mainWidget->_proposalListView->setSubModeSel(static_cast<ProposalListView::SubTextMode>(option));
}

/**************************************************************************/
void SettingsWidget::onSubModeDefChanged(int option)
{
	gSettings->setValue("subModeDeault", option);
	_mainWidget->_proposalListView->setSubModeDef(static_cast<ProposalListView::SubTextMode>(option));
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
