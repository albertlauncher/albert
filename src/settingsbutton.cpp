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

SettingsButton::SettingsButton(QWidget *parent) :
	QPushButton(parent)
{
	this->setObjectName(QString::fromLocal8Bit("settingsbutton"));
	connect(this, SIGNAL(clicked()), this, SLOT(onClick()));
}

#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <qmath.h>

void SettingsButton::paintEvent(QPaintEvent *event)
{
	QPushButton::paintEvent(event);
	QPainter p(this);
	p.drawPixmap(0,0,_icon);
	this->setFocusPolicy(Qt::NoFocus);
}

void SettingsButton::resizeEvent(QResizeEvent *event)
{
	qDebug() << "Icon resized. Is this to much?";
	_icon = QPixmap(event->size());
	_icon.fill(Qt::transparent);
	QRectF r(0, 0, event->size().width(), event->size().height());
	r.adjust(1,1,-1,-1);
	QColor cf = this->palette().brush(QPalette::Active, QPalette::WindowText).color();
	QPainter p(&_icon);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(cf);
	p.setBrush(QBrush(cf));
	p.drawEllipse(r);

	p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
	double rad=r.width()/7;
	for (int i = 0; i < 6; ++i)
	{
		QPointF c(r.center().x() + qCos(2*M_PI*i/5)*r.width()/2,
				 r.center().y() +  qSin(2*M_PI*i/5)*r.height()/2);
		p.drawEllipse(c,rad,rad);
	}
	p.drawEllipse(r.center(),rad, rad);
}

void SettingsButton::onClick()
{
	SettingsDialog::instance()->show();
}
