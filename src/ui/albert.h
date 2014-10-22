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
#include <QDebug>
#include <QFocusEvent>
//bleibt
#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QAbstractNativeEventFilter>
#include <QTime>
// meine
#include "inputline.h"
#include "proposallistview.h"
#include "albertengine.h"
#include "singleton.h"

#define gAlbertWidget AlbertWidget::instance()

class AlbertWidget : public QWidget, public Singleton<AlbertWidget>
{
	Q_OBJECT
	friend class Singleton<AlbertWidget>;

private:
	AlbertWidget(QWidget *parent = 0);
	~AlbertWidget();

	QFrame           *_frame1,*_frame2;
	InputLine        *_inputLine;
	ProposalListView *_proposalListView;
	AlbertEngine     *_engine;
	QString          _skinName;
	QTime            _t;

	void serialize() const;
	void deserialize ();

public slots:
	void onTextEdited(const QString &text);
	void show();
	void toggleVisibility();
};

#endif // ALBERT_H
