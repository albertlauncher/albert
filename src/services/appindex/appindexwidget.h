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

#ifndef APPINDEXWIDGET_H
#define APPINDEXWIDGET_H

#include "ui_appindexwidget.h"
#include "appindex.h"
#include <QWidget>

class AppIndexWidget : public QWidget
{
	Q_OBJECT
	Ui::AppIndexWidget ui;

public:
	explicit AppIndexWidget(AppIndex *fi, QWidget *parent = 0);

protected:
	AppIndex *_ref;

protected slots:
	void restoreDefaults();
	void updateUI();

	void oncb_searchTypeChanged(int);
	void onButton_PathAdd();
	void onButton_PathRemove();
	void onButton_RebuildIndex();
};

#endif // APPINDEXWIDGET_H
