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
	connect(ui.cb_searchType,SIGNAL(activated(int)),this,SLOT(oncb_searchTypeChanged(int)));
	connect(ui.pb_addPath, SIGNAL(clicked()), this, SLOT(onButton_PathAdd()));
	connect(ui.pb_removePath, SIGNAL(clicked()), this, SLOT(onButton_PathRemove()));
	connect(ui.pb_rebuildIndex, SIGNAL(clicked()), this, SLOT(onButton_RebuildIndex()));
	connect(ui.pb_restoreDefaults, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
}

/**************************************************************************/
void AppIndexWidget::oncb_searchTypeChanged(int st)
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

	_ref->_paths << pathName;
	_ref->_paths.removeDuplicates();

	// Add it in the ui
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_paths);
}

/**************************************************************************/
void AppIndexWidget::onButton_PathRemove()
{
	if (ui.lw_paths->currentItem() == nullptr)
		return;

	_ref->_paths.removeAll(ui.lw_paths->currentItem()->text());

	// Remove it in the ui
	delete ui.lw_paths->currentItem();
}

/**************************************************************************/
void AppIndexWidget::onButton_RebuildIndex()
{
	ui.lbl_info->setText("Building index...");
	ui.lbl_info->repaint();

	// Rebuild index
	_ref->buildIndex();

	// Rebuild searchindex
	_ref->setSearchType(_ref->searchType());

	ui.lbl_info->setText("Building index done.");
	QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
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
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_paths);

	// Update the search
	ui.cb_searchType->setCurrentIndex(static_cast<int>(_ref->searchType()));
}
