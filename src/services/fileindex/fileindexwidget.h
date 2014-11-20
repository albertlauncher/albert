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

#ifndef FILEINDEXWIDGET_H
#define FILEINDEXWIDGET_H

#include "ui_fileindexwidget.h"
#include "fileindex.h"
#include <QWidget>

class FileIndexWidget : public QWidget
{
	Q_OBJECT

public:
	explicit FileIndexWidget(FileIndex*, QWidget *parent = 0);

private:
	FileIndex *_index;
	Ui::FileIndexWidget ui;

protected slots:
	void onButton_AddPath();
	void onButton_RemovePath();
	void onButton_RestorePaths();
};

#endif // FILEINDEXWIDGET_H
