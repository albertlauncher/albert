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

#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class ResultWidget : public QFrame
{
	Q_OBJECT

	QHBoxLayout *_horizontalLayout;
	QLabel      *_icon;
	QVBoxLayout *_verticalLayout;
	QLabel      *_title;
	QLabel      *_auxInfo;

public:
	explicit ResultWidget(QWidget *parent = 0);
	~ResultWidget();
	inline QString title() const { return _title->text(); }
	inline QString auxInfo() const { return _auxInfo->text(); }
	inline void setTitle(const QString& t) { _title->setText(t); }
	inline void setAuxInfo(const QString& a) { _auxInfo->setText(a); }
};

#endif // RESULTWIDGET_H
