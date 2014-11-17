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

#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

/**************************************************************************/
BookmarkIndexWidget::BookmarkIndexWidget(BookmarkIndex *srv, QWidget *parent) :
	QWidget(parent), _ref(srv)
{
	ui.setupUi(this);

	//Get data from reference and initilize the ui
	updateUI();

	// Rect to changes
	connect(ui.cb_SearchType, (void (QComboBox::*)(int))&QComboBox::activated, this, &BookmarkIndexWidget::onSearchTypeChanged);
	connect(ui.pb_editPath, &QPushButton::clicked, this, &BookmarkIndexWidget::editPath);
	connect(ui.pb_rebuildIndex, &QPushButton::clicked, this, &BookmarkIndexWidget::rebuildIndex);
}

/**************************************************************************/
void BookmarkIndexWidget::onSearchTypeChanged(int st)
{
	_ref->_search.setSearchType(static_cast<Search::Type>(st));
}

/**************************************************************************/
void BookmarkIndexWidget::editPath()
{
	QString path = QFileDialog::getOpenFileName( this, tr("Choose path"),
				QFileInfo(_ref->_path).canonicalPath()); // TODO DOESNT WORK

	if(path.isEmpty())
		return;

	// TODO SANITYCHECK

	// Add it in the settings
	_ref->_path = path;

	// Add it in the ui
	ui.le_path->setText(path);
}

/**************************************************************************/
void BookmarkIndexWidget::rebuildIndex()
{
	ui.lbl_info->setText("Building index...");
	ui.lbl_info->repaint();

	// Rebuild index and searchindex
	_ref->buildIndex();

	ui.lbl_info->setText("Building index done.");
	QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
}


/**************************************************************************/
void BookmarkIndexWidget::updateUI()
{
	// Update the path
	ui.le_path->setText(_ref->_path);

	// Update the search
	ui.cb_SearchType->setCurrentIndex(static_cast<int>(_ref->_search.searchType()));
}
