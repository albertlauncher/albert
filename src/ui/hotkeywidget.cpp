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

#include "hotkeywidget.h"
#include "globalhotkey.h"
#include "globals.h"

#include <QApplication>
#include <QMessageBox>
#include <QKeySequence>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QKeyEvent>

/**************************************************************************/
HotkeyWidget::HotkeyWidget(QWidget *parent) : QLabel(parent)
{
	_waitingForHotkey = false;
	const int hk = GlobalHotkey::instance()->hotkey();
	this->setText((hk==0)
				  ? "Press to set hotkey"
				  : QKeySequence(hk).toString());

	this->setStyleSheet("padding:2px;\
						border-radius:2px;\
						background-color:#000;\
						color:#fff;");
}

/**************************************************************************/
void HotkeyWidget::grabAll()
{
	grabKeyboard();
	grabMouse();
	QApplication::setOverrideCursor(Qt::BlankCursor);
	_waitingForHotkey = true;
	GlobalHotkey::instance()->setEnabled(false);
}

/**************************************************************************/
void HotkeyWidget::releaseAll()
{
	releaseKeyboard();
	releaseMouse();
	QApplication::restoreOverrideCursor();
	_waitingForHotkey = false;
	GlobalHotkey::instance()->setEnabled(true);
}

/**************************************************************************/
void HotkeyWidget::mousePressEvent(QMouseEvent *)
{
	grabAll();
}

/**************************************************************************/
void HotkeyWidget::keyPressEvent(QKeyEvent *event)
{
	if ( _waitingForHotkey )
	{
		int currHK = GlobalHotkey::instance()->hotkey();
		int key = event->key();
		int mods = event->modifiers();

		// Modifier pressed -> update the label
		if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
			this->setText(QKeySequence(mods|Qt::Key_Question).toString());
			return;
		}

		// Cancel
		if (key == Qt::Key_Escape){
			// Reset the text
			this->setText(( currHK==0 ) ? "?" : QKeySequence(currHK).toString());
			releaseAll();
			return;
		}

		// Try to register a hotkey
		releaseAll();
		if (GlobalHotkey::instance()->registerHotkey(mods|key) )
		{
			// Fine save it..
			gSettings->setValue("hotkey", QKeySequence(mods|key).toString());
			setText(QKeySequence(mods|key).toString());
		}
		else
		{
			QMessageBox msgBox(QMessageBox::Critical, "Error",
							   QKeySequence(mods|key).toString()
							   + " could not be registered.");
			msgBox.exec();

			// Try to set the old hotkey
			if (GlobalHotkey::instance()->registerHotkey(currHK) )
			{
				gSettings->setValue("hotkey", QKeySequence(currHK).toString());
				setText(QKeySequence(currHK).toString());
				return;
			}

			// Everything failed
			gSettings->remove("hotkey");
			setText("Press to set hotkey");
		}
		return;
	}
	QWidget::keyPressEvent( event );
}

/**************************************************************************/
void HotkeyWidget::keyReleaseEvent(QKeyEvent *event)
{
	if ( _waitingForHotkey ) {
		// Modifier released -> update the label
		int key = event->key();
		if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
			this->setText(QKeySequence(event->modifiers()|Qt::Key_Question).toString());
			return;
		}
		return;
	}
	QWidget::keyReleaseEvent( event );
}
