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
	ui.setupUi(this);

	// Set current values
	ui.comboBox_searchType->setCurrentIndex(static_cast<int>(_ref->searchType()));

	// Display current paths
	ui.listWidget_paths->addItems(_ref->_paths.toList());

	// Rect to changes
	connect(ui.comboBox_searchType,SIGNAL(activated(int)),this,SLOT(onComboBox_SearchTypeChanged(int)));
	connect(ui.pushButton_addPath, SIGNAL(clicked()), this, SLOT(onButton_PathAdd()));
	connect(ui.pushButton_editPath, SIGNAL(clicked()), this, SLOT(onButton_PathEdit()));
	connect(ui.pushButton_removePath, SIGNAL(clicked()), this, SLOT(onButton_PathRemove()));
	connect(ui.pushButton_rebuildIndex, SIGNAL(clicked()), this, SLOT(onButton_RebuildIndex()));
 }

/**************************************************************************/
void FileIndexWidget::onComboBox_SearchTypeChanged(int st)
{
	ui.label_info->setText("Building search index...");
	_ref->setSearchType(static_cast<IndexService::SearchType>(st));
	ui.label_info->setText("Building search index done.");
	QTimer::singleShot(1000, ui.label_info, SLOT(clear()));
}

/**************************************************************************/
void FileIndexWidget::onButton_PathAdd()
{
	QString pathName = QFileDialog::getExistingDirectory(
				this,
				tr("Choose path"),
				QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

	if(pathName.isEmpty())
		return;

	_ref->_paths.insert(pathName);
	ui.listWidget_paths->clear();
	ui.listWidget_paths->addItems(_ref->_paths.toList());
}

/**************************************************************************/
void FileIndexWidget::onButton_PathEdit()
{
	if (ui.listWidget_paths->currentItem() == nullptr)
		return;

	QString pathName = QFileDialog::getExistingDirectory(
				this,
				tr("Choose path"),
				ui.listWidget_paths->currentItem()->text());

	if(pathName.isEmpty())
		return;

	_ref->_paths.remove(ui.listWidget_paths->currentItem()->text());
	_ref->_paths.insert(pathName);
	ui.listWidget_paths->currentItem()->setText(pathName);

}

/**************************************************************************/
void FileIndexWidget::onButton_PathRemove()
{
	if (ui.listWidget_paths->currentItem() == nullptr)
		return;

	_ref->_paths.remove(ui.listWidget_paths->currentItem()->text());
	ui.listWidget_paths->clear();
	ui.listWidget_paths->addItems(_ref->_paths.toList());
}

/**************************************************************************/
void FileIndexWidget::onButton_RebuildIndex()
{
	ui.label_info->setText("Building index...");
	_ref->buildIndex();
	ui.label_info->setText("Building index done.");
	QTimer::singleShot(1000, ui.label_info, SLOT(clear()));
}
