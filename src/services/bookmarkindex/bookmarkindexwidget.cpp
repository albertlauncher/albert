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
#include "globals.h"

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
	connect(ui.comboBoxSearchType,SIGNAL(activated(int)),this,SLOT(onSearchTypeChanged(int)));
	connect(ui.pb_editPath, SIGNAL(clicked()), this , SLOT(editPath()));
	connect(ui.pb_rebuildIndex, SIGNAL(clicked()), this , SLOT(rebuildIndex()));
	connect(ui.pb_defaults, SIGNAL(clicked()), this , SLOT(restoreDefaults()));

}

/**************************************************************************/
void BookmarkIndexWidget::onSearchTypeChanged(int st)
{
	_ref->setSearchType(static_cast<IndexService::SearchType>(st));
}

/**************************************************************************/
void BookmarkIndexWidget::editPath()
{
	QString path = QFileDialog::getOpenFileName( this, tr("Choose path"),
				QFileInfo(gSettings->value("bookmarkPath").toString()).canonicalPath()); // TODO DOESNT WORK

	if(path.isEmpty())
		return;

	// TODO SANITYCHECK

	// Add it in the settings
	gSettings->beginGroup("BookmarkIndex");
	gSettings->setValue("bookmarkPath", path);
	gSettings->endGroup();

	// Add it in the ui
	ui.le_path->setText(path);
}

/**************************************************************************/
void BookmarkIndexWidget::rebuildIndex()
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
void BookmarkIndexWidget::restoreDefaults()
{
	_ref->restoreDefaults();
	updateUI();
}

/**************************************************************************/
void BookmarkIndexWidget::updateUI()
{
	// Update the path
	gSettings->beginGroup("BookmarkIndex");
	ui.le_path->setText(gSettings->value("bookmarkPath").toString());
	gSettings->endGroup();

	// Update the search
	ui.comboBoxSearchType->setCurrentIndex(static_cast<int>(_ref->searchType()));
}
