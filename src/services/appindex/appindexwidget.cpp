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
#include "searchwidget.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

/**************************************************************************/
AppIndexWidget::AppIndexWidget(AppIndex *idx, QWidget *parent) :
	QWidget(parent), _index(idx)
{
	/* SETUP UI*/

	ui.setupUi(this);
	// Insert the settings for the search
	ui.hl_1strow->insertWidget(0, new SearchWidget(_index));


	/* INIT UI*/

	//Get data from reference and initilize the ui
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_index->paths());


	/* SETUP SIGNALS */

	// Inline oneliners
	// Show information if the index is rebuild
	connect(_index, &AppIndex::beginBuildIndex, [&](){
		ui.lbl_info->setText("Building index...");
		ui.lbl_info->repaint();
	});

	// Show information if the index has been rebuilt
	connect(_index, &AppIndex::endBuildIndex, [&](){
		ui.lbl_info->setText("Building index done.");
		QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
	});

	// Rebuild index on button press
	connect(ui.pb_rebuildIndex, &QPushButton::clicked, [&](){
		_index->buildIndex();
	});

	// Connect the explicitely implemented (long) slots
	connect(ui.pb_addPath, &QPushButton::clicked, this, &AppIndexWidget::onButton_PathAdd);
	connect(ui.pb_removePath, &QPushButton::clicked, this, &AppIndexWidget::onButton_PathRemove);
	connect(ui.pb_restorePaths, &QPushButton::clicked, this, &AppIndexWidget::onButton_RestorePaths);
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

	if (_index->addPath(pathName))
		ui.lw_paths->addItem(pathName);
	else
		qWarning("Could not add %s", pathName.toStdString().c_str());
}

/**************************************************************************/
void AppIndexWidget::onButton_PathRemove()
{
	if (ui.lw_paths->currentItem() == nullptr)
		return;

	_index->removePath(ui.lw_paths->currentItem()->text());

	// Remove it in the ui
	delete ui.lw_paths->currentItem();
}


/**************************************************************************/
void AppIndexWidget::onButton_RestorePaths()
{
	_index->restorePaths();
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_index->paths());
}

