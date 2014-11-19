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

#include "fileindexwidget.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

/**************************************************************************/
FileIndexWidget::FileIndexWidget(FileIndex *srv, QWidget *parent) :
	QWidget(parent), _ref(srv)
{
	/* SETUP UI*/

	ui.setupUi(this);
	// Insert the setting for the search
//	ui.hl_1strow->insertWidget(0, _ref->_search.widget());


	/* INIT UI*/

	// Update the list
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_paths);
	// Update the checkbox
	ui.cb_hiddenFiles->setChecked(_ref->_indexHiddenFiles);


	/* SETUP SIGNALS */

	// Inline oneliners
	// Index hidden files?
	connect(ui.cb_hiddenFiles, &QCheckBox::toggled, [&](bool b){
		_ref->_indexHiddenFiles = b;
	});

	// Rect to changes
	connect(ui.pb_add, &QPushButton::clicked, this, &FileIndexWidget::onButton_AddPath);
	connect(ui.pb_remove, &QPushButton::clicked, this, &FileIndexWidget::onButton_RemovePath);
	connect(ui.pb_restore, &QPushButton::clicked, this, &FileIndexWidget::onButton_RestorePaths);
	connect(ui.pb_rebuildIndex, &QPushButton::clicked, this, &FileIndexWidget::rebuildIndex);
 }

/**************************************************************************/
void FileIndexWidget::onButton_AddPath()
{
	QString pathName = QFileDialog::getExistingDirectory(
				this,
				tr("Choose path"),
				QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

	if(pathName.isEmpty())
		return;

	_ref->addPath(pathName);

	// Add it in the ui
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_paths);
}

/**************************************************************************/
void FileIndexWidget::onButton_RemovePath()
{
	if (ui.lw_paths->currentItem() == nullptr)
		return;

	_ref->_paths.removeAll(ui.lw_paths->currentItem()->text());

	// Remove it in the ui
	delete ui.lw_paths->currentItem();
}

/**************************************************************************/
void FileIndexWidget::onButton_RestorePaths()
{
	_ref->restorePaths();
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_paths);
}

/**************************************************************************/
void FileIndexWidget::rebuildIndex()
{
	ui.lbl_info->setText("Building index...");
	ui.lbl_info->repaint();

	// Rebuild index and searchindex
	_ref->buildIndex();

	ui.lbl_info->setText("Building index done.");
	QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
}
