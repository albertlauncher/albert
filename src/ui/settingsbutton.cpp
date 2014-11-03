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

#include "settingsbutton.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QPolygonF>
#include <qmath.h>

SettingsButton::SettingsButton(QWidget *parent) :
	QPushButton(parent)
{
	this->setObjectName(QString::fromLocal8Bit("settingsbutton"));
	this->setFocusPolicy(Qt::NoFocus);
//	setAttribute(Qt::WA_TranslucentBackground);
}


void SettingsButton::paintEvent(QPaintEvent *event)
{
	QPushButton::paintEvent(event);
	QRectF r(event->rect());
	r.adjust(1,1,-1,-1);

	QPixmap icon = QPixmap(event->rect().size());
	icon.fill(Qt::transparent);

	// Draw a simple circle TODO QTBUG-42337
	QPainter p1(&icon);
	p1.setRenderHint(QPainter::Antialiasing, true);
	p1.setPen(this->palette().brush(QPalette::Active, QPalette::WindowText).color());
	p1.setBrush(QBrush(this->palette().brush(QPalette::Active, QPalette::WindowText).color()));
	p1.drawEllipse(r);

	// Cut the cogs of the wheel
	p1.setCompositionMode(QPainter::CompositionMode_DestinationOut);
	double rad=r.width()/7;
	for (int i = 0; i < 6; ++i)
	{
		QPointF c(r.center().x() + qCos(2*M_PI*i/5)*r.width()/2,
				 r.center().y() +  qSin(2*M_PI*i/5)*r.height()/2);
		p1.drawEllipse(c,rad,rad);
	}
	p1.drawEllipse(r.center(),rad, rad);

	QPainter p2(this);
	p2.drawPixmap(0,0,icon);
}
