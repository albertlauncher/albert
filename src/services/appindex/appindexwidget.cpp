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
#include <QComboBox>

/**************************************************************************/
AppIndexWidget::AppIndexWidget(AppIndex *srv, QWidget *parent) :
	QWidget(parent), _ref(srv)
{
	ui.setupUi(this);

	//Get data from reference and initilize the ui
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_watcher.directories());
	ui.cb_searchType->setCurrentIndex(static_cast<int>(_ref->_search.searchType()));

	/* Connect all the onliners */

	// Show information if the index is rebuild
	connect(_ref, &AppIndex::beginBuildIndex, [=](){
		ui.lbl_info->setText("Building index...");
	});

	// Show information if the index has been rebuilt
	connect(_ref, &AppIndex::endBuildIndex, [=](){
		ui.lbl_info->setText("Building index done.");
		QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
	});

	// Change the searchtype on selection
	connect(ui.cb_searchType, (void (QComboBox::*)(int))&QComboBox::activated, [=](int st){
		_ref->_search.setSearchType(static_cast<Search::Type>(st));
	});

	// Rebuild index on button press
	connect(ui.pb_rebuildIndex, &QPushButton::clicked, [=](){
		_ref->buildIndex();
	});

	// Connect the explicitely implemented (long) slots
	connect(ui.pb_addPath, SIGNAL(clicked()), this, SLOT(onButton_PathAdd()));
	connect(ui.pb_removePath, SIGNAL(clicked()), this, SLOT(onButton_PathRemove()));
	connect(ui.pb_restorePaths, SIGNAL(clicked()), this, SLOT(onButton_RestorePaths()));
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

	if (_ref->_watcher.addPath(pathName))
		ui.lw_paths->addItem(pathName);
	else
		qWarning("Could not add %s", pathName.toStdString().c_str());
}

/**************************************************************************/
void AppIndexWidget::onButton_PathRemove()
{
	if (ui.lw_paths->currentItem() == nullptr)
		return;

	_ref->_watcher.removePath(ui.lw_paths->currentItem()->text());

	// Remove it in the ui
	delete ui.lw_paths->currentItem();
}


/**************************************************************************/
void AppIndexWidget::onButton_RestorePaths()
{
	_ref->restorePaths();
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_watcher.directories());
}

