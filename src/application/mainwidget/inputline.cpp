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
#include <QDir>
#include <QDebug>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QBoxLayout>

/** ***************************************************************************/
InputLine::InputLine(QWidget *parent) : QLineEdit(parent) {

    // This means historymode is not active
    _currentLine = _lines.crend();
    connect(this, &QLineEdit::textEdited, this, &InputLine::resetIterator);

    // Add setting button overlay
    _settingsButton = new SettingsButton(this);
    _settingsButton->setObjectName("settingsButton");
    _settingsButton->setFocusPolicy(Qt::NoFocus);
    _settingsButton->setShortcut(QKeySequence(SETTINGS_SHORTCUT));
    _settingsButton->setCursor(QCursor(Qt::ArrowCursor));

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    this->setLayout(hlayout);
    hlayout->addStretch();
    hlayout->setMargin(0);
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    hlayout->addLayout(vlayout);
    vlayout->setMargin(0);
    vlayout->addWidget(_settingsButton);
    vlayout->addStretch();

    // DESERIALIZATION
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("history.dat")));
    if (dataFile.exists())
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            QDataStream in(&dataFile);
            QStringList SL;
            in >> SL;
            _lines = SL.toStdList();
            dataFile.close();
        } else qWarning() << "Could not open file" << dataFile.fileName();
}



/** ***************************************************************************/
InputLine::~InputLine() {
    // SERIALIZATION
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("history.dat")));
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        QDataStream out(&dataFile);
        out << QStringList::fromStdList(_lines);
        dataFile.close();
    } else qCritical() << "Could not write to " << dataFile.fileName();

    _settingsButton->deleteLater();
}



/** ***************************************************************************/
void InputLine::clear() {
    resetIterator();
    QLineEdit::clear();
}



/** ***************************************************************************/
void InputLine::resetIterator() {
    _currentLine = _lines.crend();
}



/** ***************************************************************************/
void InputLine::next() {
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
void InputLine::prev() {
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
void InputLine::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Up:
        next();
        return;
    case Qt::Key_Down:
        prev();
        return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!text().isEmpty()) {
            _lines.remove(text()); // Make entries uniq
            _lines.push_back(text()); // Remember this entry
        }
        break;
    }
    e->ignore();
    QLineEdit::keyPressEvent(e);
}



/** ***************************************************************************/
void InputLine::wheelEvent(QWheelEvent *e) {
    e->angleDelta().ry()<0 ? prev() : next();
}
