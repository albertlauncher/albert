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

#include "appindexwidget.h"
#include "globals.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

/**************************************************************************/
AppIndexWidget::AppIndexWidget(AppIndex *srv, QWidget *parent) :
	QWidget(parent), _ref(srv)
{
	ui.setupUi(this);

	//Get data from reference and initilize the ui
	updateUI();

	// Rect to changes
	connect(ui.comboBox_searchType,SIGNAL(activated(int)),this,SLOT(onComboBox_SearchTypeChanged(int)));
	connect(ui.pushButton_addPath, SIGNAL(clicked()), this, SLOT(onButton_PathAdd()));
	connect(ui.pushButton_removePath, SIGNAL(clicked()), this, SLOT(onButton_PathRemove()));
	connect(ui.pushButton_rebuildIndex, SIGNAL(clicked()), this, SLOT(onButton_RebuildIndex()));
	connect(ui.pb_defaults, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
}

/**************************************************************************/
void AppIndexWidget::onComboBox_SearchTypeChanged(int st)
{
	_ref->setSearchType(static_cast<IndexService::SearchType>(st));
}

/**************************************************************************/
void AppIndexWidget::onButton_PathAdd()
{
	QString pathName = QFileDialog::getExistingDirectory(
				this,
				tr("Choose path"),
				QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

	if(pathName.isEmpty())
		return;

	// Add it in the settings
	gSettings->beginGroup("AppIndex");
	QStringList paths = gSettings->value("paths", "").toStringList();
	paths << pathName;
	paths.removeDuplicates();
	gSettings->setValue("paths", paths);
	gSettings->endGroup();

	// Add it in the ui
	ui.listWidget_paths->clear();
	ui.listWidget_paths->addItems(paths);
}

/**************************************************************************/
void AppIndexWidget::onButton_PathRemove()
{
	if (ui.listWidget_paths->currentItem() == nullptr)
		return;

	// Remove it in the settings
	gSettings->beginGroup("AppIndex");
	QStringList paths = gSettings->value("paths", "").toStringList();
	paths.removeAll(ui.listWidget_paths->currentItem()->text());
	gSettings->setValue("paths", paths);
	gSettings->endGroup();

	// Remove it in the ui
	ui.listWidget_paths->clear();
	ui.listWidget_paths->addItems(paths);

	// Select first
	if(ui.listWidget_paths->count() > 0)
		ui.listWidget_paths->setCurrentRow(0);
}

/**************************************************************************/
void AppIndexWidget::onButton_RebuildIndex()
{
	ui.label_info->setText("Building index...");
	ui.label_info->repaint();

	// Rebuild index
	_ref->buildIndex();

	// Rebuild searchindex
	_ref->setSearchType(_ref->searchType());

	ui.label_info->setText("Building index done.");
	QTimer::singleShot(1000, ui.label_info, SLOT(clear()));
}

/**************************************************************************/
void AppIndexWidget::restoreDefaults()
{
	_ref->restoreDefaults();
	updateUI();
}

/**************************************************************************/
void AppIndexWidget::updateUI()
{
	// Update the list
	gSettings->beginGroup("AppIndex");
	QStringList paths = gSettings->value("paths", "").toStringList();
	gSettings->endGroup();
	ui.listWidget_paths->clear();
	ui.listWidget_paths->addItems(paths);

	// Update the search
	ui.comboBox_searchType->setCurrentIndex(static_cast<int>(_ref->searchType()));
}
