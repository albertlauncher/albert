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

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>


/**************************************************************************/
SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);


	/* GENERAL */

	// hotkey setter
	_hkWidget = new HotkeyWidget;
	ui.tabGeneral->layout()->addWidget(_hkWidget);


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


	/* MODULES */

	QListWidgetItem *item = new QListWidgetItem("Websearch");
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(WebSearch::instance()->widget());

	item = new QListWidgetItem("Apps");
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(AppIndex::instance()->widget());

	item = new QListWidgetItem("Bookmarks");
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(BookmarkIndex::instance()->widget());

	item = new QListWidgetItem("Files");
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(FileIndex::instance()->widget());

	item = new QListWidgetItem("Calculator");
	ui.listWidget->addItem(item);
	ui.stackedWidget->addWidget(Calculator::instance()->widget());

	ui.listWidget->setCurrentRow(0);
	ui.listWidget->adjustSize();
}

/**************************************************************************/
void SettingsDialog::onSkinClicked(QListWidgetItem *i)
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
