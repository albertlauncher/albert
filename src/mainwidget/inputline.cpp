// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include <QResizeEvent>
#include <QDebug>
#include <QKeyEvent>

/** ***************************************************************************/
InputLine::InputLine(QWidget *parent) : QLineEdit(parent)
{
    _settingsButton = new SettingsButton(this);
    _settingsButton->setObjectName("settingsbutton");
    _settingsButton->setFocusPolicy(Qt::NoFocus);
    _settingsButton->setShortcut(QKeySequence(SETTINGS_SHORTCUT));

    _currentLine = _lines.crend(); // This means historymode is not active

    connect(this, &QLineEdit::textEdited,
            this, &InputLine::resetIterator);
}

/** ***************************************************************************/
InputLine::~InputLine()
{
    _settingsButton->deleteLater();
}

/** ***************************************************************************/
void InputLine::clear()
{
    resetIterator();
    QLineEdit::clear();
}

/** ***************************************************************************/
void InputLine::resetIterator()
{
    _currentLine = _lines.crend();
}

/** ***************************************************************************/
void InputLine::next()
{
    if ( _lines.empty() ) // (1) implies _lines.crbegin() !=_lines.crend()
        return;

    if (_currentLine == _lines.crend()) // Not in history mode
        _currentLine = _lines.crbegin(); // This may still be crend!
    else
        if (++_currentLine == _lines.crend())
            --_currentLine;
    setText(*_currentLine);
}

/** ***************************************************************************/
void InputLine::prev()
{
    if ( _lines.empty() ) // (1) implies _lines.crbegin() !=_lines.crend()
        return;

    if (_currentLine == _lines.crend()) // Not in history mode
        _currentLine = _lines.crbegin(); // This may still be crend!
    else
        if (_currentLine != _lines.crbegin())
            --_currentLine;
    setText(*_currentLine);
}


/** ***************************************************************************/
void InputLine::keyPressEvent(QKeyEvent *e)
{
    qDebug() << QKeySequence(e->modifiers()|e->key()).toString();
    switch (e->key()) {
    case Qt::Key_Up:
        next();
        return;
    case Qt::Key_Down:
        prev();
        return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!text().isEmpty()){
            _lines.remove(text()); // Make entries uniq
            _lines.push_back(text()); // Remember this entry
        }
        break;
    }
    e->ignore();
    QLineEdit::keyPressEvent(e);
}

/** ***************************************************************************/
void InputLine::wheelEvent(QWheelEvent *e)
{
    e->angleDelta().ry()<0 ? prev() : next();
}

/** ***************************************************************************/
void InputLine::resizeEvent(QResizeEvent *event)
{
    //Let settingsbutton be in top right corner
    _settingsButton->move(event->size().width()-_settingsButton->width(),0);
}
