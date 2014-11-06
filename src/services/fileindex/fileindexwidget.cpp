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
#include "globals.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

/**************************************************************************/
FileIndexWidget::FileIndexWidget(FileIndex *srv, QWidget *parent) :
	QWidget(parent), _ref(srv)
{
	ui.setupUi(this);

	// Init ui
	ui.cb_searchType->setCurrentIndex(static_cast<int>(_ref->searchType()));
	ui.cb_hiddenFiles->setChecked(gSettings->value("indexHiddenFiles", false).toBool());
	ui.lw_paths->addItems(_ref->_paths);

	// Rect to changes
	connect(ui.cb_searchType,SIGNAL(activated(int)),this,SLOT(oncb_searchTypeChanged(int)));
	connect(ui.pb_add, SIGNAL(clicked()), this, SLOT(onButton_add()));
	connect(ui.pb_edit, SIGNAL(clicked()), this, SLOT(onButton_edit()));
	connect(ui.pb_remove, SIGNAL(clicked()), this, SLOT(onButton_remove()));
	connect(ui.pb_restore, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
	connect(ui.pb_rebuildIndex, SIGNAL(clicked()), this, SLOT(rebuildIndex()));
	connect(ui.cb_hiddenFiles, SIGNAL(toggled(bool)), this, SLOT(onCheckbox_toggle(bool)));
 }

/**************************************************************************/
void FileIndexWidget::oncb_searchTypeChanged(int st)
{
	ui.lbl_info->setText("Building search index...");
	_ref->setSearchType(static_cast<IndexService::SearchType>(st));
	ui.lbl_info->setText("Building search index done.");
	QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
}

/**************************************************************************/
void FileIndexWidget::onButton_add()
{
	QString pathName = QFileDialog::getExistingDirectory(
				this,
				tr("Choose path"),
				QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

	if(pathName.isEmpty())
		return;

	_ref->_paths.append(pathName);
	ui.lw_paths->clear();
	ui.lw_paths->addItems(_ref->_paths);
}

/**************************************************************************/
void FileIndexWidget::onButton_edit()
{
	if (ui.lw_paths->currentItem() == nullptr)
		return;

	QString pathName = QFileDialog::getExistingDirectory(
				this,
				tr("Choose path"),
				ui.lw_paths->currentItem()->text());

	if(pathName.isEmpty())
		return;

	_ref->_paths.removeAt(ui.lw_paths->currentRow());
	_ref->_paths.append(pathName);
	ui.lw_paths->currentItem()->setText(pathName);

}

/**************************************************************************/
void FileIndexWidget::onButton_remove()
{
	if (ui.lw_paths->currentItem() == nullptr)
		return;
	delete ui.lw_paths->currentItem();
	_ref->_paths.removeAt(ui.lw_paths->currentRow());
}

/**************************************************************************/
void FileIndexWidget::rebuildIndex()
{
	ui.lbl_info->setText("Building index...");
	ui.lbl_info->repaint();

	// Rebuild index
	_ref->buildIndex();

	// Rebuild searchindex (id necessary)
	_ref->setSearchType(_ref->searchType());

	ui.lbl_info->setText("Building index done.");
	QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
}

/**************************************************************************/
void FileIndexWidget::onCheckbox_toggle(bool b)
{
	gSettings->setValue("indexHiddenFiles", b);
}

/**************************************************************************/
void FileIndexWidget::restoreDefaults()
{
	_ref->restoreDefaults();
	updateUI();
}

/**************************************************************************/
void FileIndexWidget::updateUI()
{
	// Update the list
	gSettings->beginGroup("FileIndex");
	QStringList paths = gSettings->value("paths", "").toStringList();
	gSettings->endGroup();
	ui.lw_paths->clear();
	ui.lw_paths->addItems(paths);

	// Update the search
	ui.cb_searchType->setCurrentIndex(static_cast<int>(_ref->searchType()));
}


