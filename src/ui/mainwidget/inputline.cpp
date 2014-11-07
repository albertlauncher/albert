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

#include "inputline.h"
#include "settingsdialog.h"
#include <QString>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QResizeEvent>

/**************************************************************************/
InputLine::InputLine(QWidget *parent) : QLineEdit(parent)
{
	_settingsButton = new SettingsButton(this);
	_settingsButton->setFocusPolicy(Qt::NoFocus);
	connect(_settingsButton, SIGNAL(clicked()), this, SIGNAL(settingsDialogRequested()));
}

/**************************************************************************/
InputLine::~InputLine()
{
	delete _settingsButton;
}

/**************************************************************************/
void InputLine::resizeEvent(QResizeEvent *event)
{
	_settingsButton->move(event->size().width()-_settingsButton->width(),0);
}

/**************************************************************************/
void InputLine::keyPressEvent(QKeyEvent *e)
{
	// Open settings dialog
	if (e->modifiers() == Qt::AltModifier && e->key() == Qt::Key_Comma ) {
		emit settingsDialogRequested();
		return;
	}

	// Quit application
	if (/*e->modifiers() == Qt::AltModifier && */e->key() == Qt::Key_F4 ) {
		window()->hide();
		qApp->quit();
		return;
	}

	// Hide window
	if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Escape ) {
		window()->hide();
		return;
	}

	QLineEdit::keyPressEvent(e);
}
