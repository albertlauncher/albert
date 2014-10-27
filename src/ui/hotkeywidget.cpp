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
#include <QApplication>
#include <QMessageBox>
#include <QKeySequence>
#include <QMouseEvent>
#include <QKeyEvent>

/**************************************************************************/
HotkeyWidget::HotkeyWidget(QWidget *parent) : QLabel(parent)
{
	_settingHotkey = false;
	this->setObjectName("hotkeyWidget");
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	const GlobalHotkey::Hotkey HK = GlobalHotkey::instance()->hotkey();
	this->setText((HK._key == Qt::Key(0)) ? "?" : keyKomboToString(HK._mods,HK._key));
}


/**************************************************************************/
QString HotkeyWidget::keyKomboToString(Qt::KeyboardModifiers mod, int key)
{
	if(mod & Qt::ShiftModifier)
		key += Qt::SHIFT;
	if(mod & Qt::ControlModifier)
		key += Qt::CTRL;
	if(mod & Qt::AltModifier)
		key += Qt::ALT;
	if(mod & Qt::MetaModifier)
		key += Qt::META;
	return QKeySequence(key).toString(QKeySequence::NativeText);
}

/**************************************************************************/
void HotkeyWidget::grabAll()
{
	grabKeyboard();
	grabMouse();
	QApplication::setOverrideCursor(Qt::BlankCursor);
	_settingHotkey = true;
	GlobalHotkey::instance()->setEnabled(false);
}

/**************************************************************************/
void HotkeyWidget::releaseAll()
{
	releaseKeyboard();
	releaseMouse();
	QApplication::restoreOverrideCursor();
	_settingHotkey = false;
	GlobalHotkey::instance()->setEnabled(true);
}

/**************************************************************************/
void HotkeyWidget::mousePressEvent(QMouseEvent *)
{
	grabAll();
	GlobalHotkey::instance()->setEnabled(false);
}

/**************************************************************************/
void HotkeyWidget::keyPressEvent(QKeyEvent *event)
{
	if ( _settingHotkey )
	{
		int key = event->key();
		Qt::KeyboardModifiers mods = event->modifiers();

		// Modifier pressed -> update the label
		if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
			this->setText(keyKomboToString(mods, Qt::Key_Question));
			return;
		}

		// Cancel
		if (key == Qt::Key_Escape){
			// Reset the text
			const GlobalHotkey::Hotkey HK = GlobalHotkey::instance()->hotkey();
			this->setText((HK._key == Qt::Key(0)) ? "?" : keyKomboToString(HK._mods,HK._key));
			releaseAll();
			return;
		}

		// Try to register a hotkey
		releaseAll();
		if ( !GlobalHotkey::instance()->setHotkey({mods, Qt::Key(key)}) ) {
			QMessageBox msgBox(QMessageBox::Critical, "Error",
							   keyKomboToString(mods, key) +
							   " could not be added.");
			msgBox.exec();
			return;
		}

		// Fine show it..
		this->setText(keyKomboToString(mods, key));
		return;
	}
	QWidget::keyPressEvent( event );
}

/**************************************************************************/
void HotkeyWidget::keyReleaseEvent(QKeyEvent *event)
{
	if ( _settingHotkey ) {
		// Modifier released -> update the label
		int key = event->key();
		if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
			this->setText(keyKomboToString(event->modifiers(), Qt::Key_Question));
			return;
		}
		return;
	}
	QWidget::keyReleaseEvent( event );
}
