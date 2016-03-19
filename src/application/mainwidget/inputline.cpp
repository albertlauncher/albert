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

/** ***************************************************************************/
InputLine::InputLine(QWidget *parent) : QLineEdit(parent) {
    settingsButton_ = new SettingsButton(this);
    settingsButton_->setObjectName("settingsButton");
    settingsButton_->setFocusPolicy(Qt::NoFocus);
    settingsButton_->setShortcut(QKeySequence(SETTINGS_SHORTCUT));

    currentLine_ = lines_.crend(); // This means historymode is not active

    connect(this, &QLineEdit::textEdited, this, &InputLine::resetIterator);

    // DESERIALIZATION
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("history.dat")));
    if (dataFile.exists()){
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            QDataStream in(&dataFile);
            QStringList SL;
            in >> SL;
            lines_ = SL.toStdList();
            dataFile.close();
        } else qWarning() << "Could not open file" << dataFile.fileName();
    }
}



/** ***************************************************************************/
InputLine::~InputLine() {
    // SERIALIZATION
    QFile dataFile(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).
                   filePath(QString("history.dat")));
    if (dataFile.open(QIODevice::ReadWrite| QIODevice::Text)) {
        QDataStream out(&dataFile);
        out << QStringList::fromStdList(lines_);
        dataFile.close();
    } else qCritical() << "Could not write to " << dataFile.fileName();

    settingsButton_->deleteLater();
}



/** ***************************************************************************/
void InputLine::clear() {
    resetIterator();
    QLineEdit::clear();
}



/** ***************************************************************************/
void InputLine::resetIterator() {
    currentLine_ = lines_.crend();
}



/** ***************************************************************************/
void InputLine::next() {
    if ( lines_.empty() ) // (1) implies lines.crbegin() !=_lines.crend()
        return;

    if (currentLine_ == lines_.crend()) // Not in history mode
        currentLine_ = lines_.crbegin(); // This may still be crend!
    else
        if (++currentLine_ == lines_.crend())
            --currentLine_;
    setText(*currentLine_);
}



/** ***************************************************************************/
void InputLine::prev() {
    if ( lines_.empty() ) // (1) implies lines.crbegin() !=_lines.crend()
        return;

    if (currentLine_ == lines_.crend()) // Not in history mode
        currentLine_ = lines_.crbegin(); // This may still be crend!
    else
        if (currentLine_ != lines_.crbegin())
            --currentLine_;
    setText(*currentLine_);
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
            lines_.remove(text()); // Make entries uniq
            lines_.push_back(text()); // Remember this entry
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



/** ***************************************************************************/
void InputLine::resizeEvent(QResizeEvent *event) {
    //Let settingsbutton be in top right corner
    settingsButton_->move(event->size().width()-settingsButton_->width(),0);
}
