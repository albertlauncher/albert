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

#include <QWidget>
#include "inputline.h"
#include "proposallistview.h"

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = 0);
	~MainWidget();

	InputLine        *_inputLine;
	ProposalListView *_proposalListView;

protected:
	bool nativeEvent(const QByteArray &eventType, void *message, long *) override;

private:
	QFrame  *_frame1,*_frame2;
	QString _theme;

signals:
	void widgetShown();
	void widgetHidden();

public slots:
	void show();
	void hide();
	void toggleVisibility();
};

#endif // ALBERT_H
