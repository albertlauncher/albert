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
#include "searchwidget.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

/**************************************************************************/
FileIndexWidget::FileIndexWidget(FileIndex *srv, QWidget *parent) :
	QWidget(parent), _index(srv)
{
	/* SETUP UI*/

	ui.setupUi(this);
	// Insert the setting for the search
	ui.hl_1strow->insertWidget(0, new SearchWidget(_index));


	/* INIT UI*/

	// Update the list
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_index->paths());
	// Update the checkbox
	ui.cb_hiddenFiles->setChecked(_index->indexHiddenFiles());


	/* SETUP SIGNALS */

	// Inline oneliners
	// Index hidden files?
	connect(ui.cb_hiddenFiles, &QCheckBox::toggled, [&](bool b){
		_index->setIndexHiddenFiles(b);
	});

	// Show information if the index is rebuild
	connect(_index, &FileIndex::beginBuildIndex, [&](){
		ui.lbl_info->setText("Building index...");
	});

	// Show information if the index has been rebuilt
	connect(_index, &FileIndex::endBuildIndex, [&](){
		ui.lbl_info->setText("Building index done.");
		QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
	});

	// Rebuild index on button press
	connect(ui.pb_rebuildIndex, &QPushButton::clicked, [&](){
		_index->buildIndex();
	});

	// Rect to changes
	connect(ui.pb_add, &QPushButton::clicked, this, &FileIndexWidget::onButton_AddPath);
	connect(ui.pb_remove, &QPushButton::clicked, this, &FileIndexWidget::onButton_RemovePath);
	connect(ui.pb_restore, &QPushButton::clicked, this, &FileIndexWidget::onButton_RestorePaths);
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

	_index->addPath(pathName);
	// Add it in the ui
	ui.lw_paths->addItem(pathName);
}

/**************************************************************************/
void FileIndexWidget::onButton_RemovePath()
{
	if (ui.lw_paths->currentItem() == nullptr)
		return;

	_index->removePath(ui.lw_paths->currentItem()->text());

	// Remove it in the ui
	delete ui.lw_paths->currentItem();
}

/**************************************************************************/
void FileIndexWidget::onButton_RestorePaths()
{
	_index->restorePaths();
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_index->paths());
}
