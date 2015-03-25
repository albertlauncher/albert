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

#include "grabkeybutton.h"

/****************************************************************************///

GrabKeyButton::GrabKeyButton(QWidget * parent) : QPushButton(parent)
{
    _waitingForHotkey = false;
    connect(this, &QPushButton::clicked,
            this, &GrabKeyButton::onClick);
}

/****************************************************************************///
GrabKeyButton::~GrabKeyButton()
{    
}

/****************************************************************************///
void GrabKeyButton::onClick()
{
    setText("?");
    grabAll();
}

/****************************************************************************///
void GrabKeyButton::grabAll()
{
    grabKeyboard();
    grabMouse();
    _waitingForHotkey = true;
}

/****************************************************************************///
void GrabKeyButton::releaseAll()
{
    releaseKeyboard();
    releaseMouse();
    _waitingForHotkey = false;
}

/****************************************************************************///
void GrabKeyButton::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    int mods = event->modifiers();

    if ( _waitingForHotkey )
    {
        // Modifier pressed -> update the label
        if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
            setText(QKeySequence(mods|Qt::Key_Question).toString());
            return;
        }

        releaseAll();
        setText(QKeySequence(mods|key).toString());
        emit keyCombinationPressed(mods|key);
        return;
    }

    QWidget::keyPressEvent( event );
}

/****************************************************************************///
void GrabKeyButton::keyReleaseEvent(QKeyEvent *event)
{
    if ( _waitingForHotkey )
    {
        // Modifier released -> update the label
        int key = event->key();
        if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
            setText(QKeySequence(event->modifiers()|Qt::Key_Question).toString());
            return;
        }
        return;
    }
    QWidget::keyReleaseEvent( event );
}
