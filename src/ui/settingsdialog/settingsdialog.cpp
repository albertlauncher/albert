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

	// hotkey setter
	_hkWidget = new HotkeyWidget; // Initializes itself
	ui.groupBox_hotkey->layout()->addWidget(_hkWidget);

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



	/* MODULES */

	QListWidgetItem *item = new QListWidgetItem("Websearch"); // TODO NO SINGLETONS!!!
	ui.listWidget_modules->addItem(item);
	ui.stackedWidget->addWidget(WebSearch::instance()->widget());

	item = new QListWidgetItem("Apps");
	ui.listWidget_modules->addItem(item);
	ui.stackedWidget->addWidget(AppIndex::instance()->widget());

	item = new QListWidgetItem("Bookmarks");
	ui.listWidget_modules->addItem(item);
	ui.stackedWidget->addWidget(BookmarkIndex::instance()->widget());

	item = new QListWidgetItem("Files");
	ui.listWidget_modules->addItem(item);
	ui.stackedWidget->addWidget(FileIndex::instance()->widget());

	item = new QListWidgetItem("Calculator");
	ui.listWidget_modules->addItem(item);
	ui.stackedWidget->addWidget(Calculator::instance()->widget());

	// The entries and the stackeswidgets are connected in the .ui

	// Set the width of the list to the with of the content
	QFontMetrics fm(ui.listWidget_modules->font());
	int width=0;
	for (int i = 0; i < ui.listWidget_modules->count(); ++i)
	{
		int tmp = fm.width(ui.listWidget_modules->item(i)->text());
		if (width < tmp)
			width = tmp;
	}
	ui.listWidget_modules->setFixedWidth(width
										 + ui.listWidget_modules->contentsMargins().left()
										 + ui.listWidget_modules->contentsMargins().right()
										 + 6
										 );



	/* APPEARANCE */

	// Add a simple fallback without path
	ui.listWidget_skins->addItem("Standard (Fallback)");

	// Get theme dirs
	QStringList themeDirs = QStandardPaths::locateAll(QStandardPaths::DataLocation,
													  "themes",
													  QStandardPaths::LocateDirectory);
	// Get all themes
	QFileInfoList themes;
	for (QDir d : themeDirs)
		themes << d.entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);

	// Add the themes
	for (QFileInfo fi : themes)
	{
		ui.listWidget_skins->addItem(fi.baseName());
		ui.listWidget_skins->item(ui.listWidget_skins->count()-1)->setData(
					Qt::UserRole,
					fi.canonicalFilePath());
	}

	// Apply a skin if clicked
	connect(ui.listWidget_skins, SIGNAL(itemClicked(QListWidgetItem*)),
			this, SLOT(onSkinClicked(QListWidgetItem*)));
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
void SettingsWidget::onSkinClicked(QListWidgetItem *i)
{
	// Apply and save the theme
	QString path(i->data(Qt::UserRole).toString());
	if (path.isEmpty())
	{
		qApp->setStyleSheet(QString::fromLocal8Bit("file:///:/resources/Standard.qss"));
		gSettings->remove("theme");
	}
	else
	{
		QFile styleFile(path);
		if (styleFile.open(QFile::ReadOnly)) {
			qApp->setStyleSheet(styleFile.readAll());
			gSettings->setValue("theme", path.section('/', -1));
			styleFile.close();
		}
		else
		{
			QMessageBox msgBox(QMessageBox::Critical, "Error",
							   "Could not open theme file.");
			msgBox.exec();
		}
	}
}

/**************************************************************************/
void SettingsWidget::onNItemsChanged(int i)
{
	gSettings->setValue("nItemsToShow", i);
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
