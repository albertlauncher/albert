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

#ifndef ALBERT_H
#define ALBERT_H


// aufräumemn
#include <QDesktopWidget>
#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QAbstractNativeEventFilter>

//bleibtz
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QList>
#include "commandline.h"
#include "albertengine.h"
#include "listitemwidget.h"

class AlbertWidget : public QWidget
{
	Q_OBJECT

public:
	AlbertWidget(QWidget *parent = 0);
	~AlbertWidget();
private slots:
	void onHotKeyPressed();
	void onReturnPressed();
	void hideAndClear();
	void onTextEdited(const QString &text);
protected:
	void         keyPressEvent(QKeyEvent * event) override;
	bool         eventFilter(QObject *obj, QEvent *event) override;
	virtual bool nativeEvent(const QByteArray &eventType, void *message, long *) override;
private:
	CommandLine	  * _commandLine;
	QList<ListItemWidget*>  _resultsWidgetList;
	AlbertEngine  _engine;
	int           _nItemsToShow;
	QVBoxLayout   *_contentLayout;
};

#endif // ALBERT_H


/*
 * Calc
 * Desktop
 * Binary
 * Files
 * Dirs
 */
