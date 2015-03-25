// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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

#include "configwidget.h"
//#include "searchwidget.h"
#include <QFileDialog>
#include <QStandardPaths>
#include "settings.h"

///****************************************************************************///
//ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent)
//{
//	dirtyFlag = false;
//	ui.setupUi(this);
//	// Insert the settings for the search
////	ui.hl_1strow->insertWidget(0, new SearchWidget(_ref));

//	ui.lw_paths->addItems(gSettings->value("AppIndex/Paths", "").toStringList());


//	/* SETUP SIGNALS */

//	// Inline oneliners
////	// Show information if the index is rebuild
////	connect(_ref, &AppIndex::beginBuildIndex, [&](){
////		ui.lbl_info->setText("Building index...");
////		ui.lbl_info->repaint();
////	});

////	// Show information if the index has been rebuilt
////	connect(_ref, &AppIndex::endBuildIndex, [&](){
////		ui.lbl_info->setText("Building index done.");
////		QTimer::singleShot(1000, ui.lbl_info, SLOT(clear()));
////	});

////	// Rebuild index on button press
////	connect(ui.pb_rebuildIndex, &QPushButton::clicked, [&](){
////		_ref->buildIndex();
////	});

////	// Connect the explicitely implemented (long) slots
////	connect(ui.pb_addPath, &QPushButton::clicked,this, &ConfigWidget::onButton_PathAdd);
////	connect(ui.pb_removePath, &QPushButton::clicked, this, &ConfigWidget::onButton_PathRemove);
////	connect(ui.pb_restorePaths, &QPushButton::clicked, this, &ConfigWidget::onButton_RestorePaths);
//}

///****************************************************************************///
//void ConfigWidget::onButton_PathAdd()
//{
//	QString pathName = QFileDialog::getExistingDirectory(
//				this,
//				tr("Choose path"),
//				QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
//	if(pathName.isEmpty())return;
//	ui.lw_paths->addItem(pathName);
//}

///****************************************************************************///
//void ConfigWidget::onButton_PathRemove()
//{
//	if (ui.lw_paths->currentItem() == nullptr) return;
//	delete ui.lw_paths->currentItem();
//}

///****************************************************************************///
//void ConfigWidget::onButton_RestorePaths()
//{
//	ui.lw_paths->clear();
//}

