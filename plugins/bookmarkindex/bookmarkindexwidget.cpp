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

#include "bookmarkindexwidget.h"
#include "searchwidget.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>
#include <QMessageBox>

/**************************************************************************/
BookmarkIndexWidget::BookmarkIndexWidget(BookmarkIndex *idx, QWidget *parent) :
	QWidget(parent), _index(idx)
{
	/* SETUP UI*/

	ui.setupUi(this);
	// Insert the setting for the search
	ui.hl_1strow->insertWidget(0, new SearchWidget(_index));


	/* INIT UI*/

	//Get data from reference and initilize the ui
	ui.le_path->setText(_index->path());


	/* SETUP SIGNALS */

	// Inline oneliners
	// Show information if the index is rebuild
	connect(_index, &BookmarkIndex::beginBuildIndex, [&](){
		ui.lbl_info->setText("Building index...");
		ui.lbl_info->repaint();
	});

	// Show information if the index has been rebuilt
	connect(_index, &BookmarkIndex::endBuildIndex, [&](){
		ui.lbl_info->setText("Building index done.");
		QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
	});

	// Rebuild index on button press
	connect(ui.pb_rebuildIndex, &QPushButton::clicked, [&](){
		_index->buildIndex();
	});

	// Connect the explicitely implemented (long) slots
	connect(ui.pb_editPath, &QPushButton::clicked,
			this, &BookmarkIndexWidget::onButton_EditPath);
}

/**************************************************************************/
void BookmarkIndexWidget::onButton_EditPath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Choose path"),_index->path());

	if(path.isEmpty())
		return;

	// Add it in the settings
	if (!_index->setPath(path)){
		QMessageBox msgBox(QMessageBox::Warning, "Warning",
						   "The path to the bookmarks file is invalid");
		msgBox.exec();
	}

	ui.le_path->setText(_index->path());
}
