// Copyright (C) 2014-2018 Manuel Schneider

#include "grabkeybutton.h"

/** ***************************************************************************/

Core::GrabKeyButton::GrabKeyButton(QWidget * parent) : QPushButton(parent) {
    waitingForHotkey_ = false;
    connect(this, &QPushButton::clicked,
            this, &GrabKeyButton::onClick);
}



/** ***************************************************************************/
Core::GrabKeyButton::~GrabKeyButton() {
}



/** ***************************************************************************/
void Core::GrabKeyButton::onClick() {
    oldText_ = text();
    setText("?");
    grabAll();
}



/** ***************************************************************************/
void Core::GrabKeyButton::grabAll() {
    grabKeyboard();
    grabMouse();
    waitingForHotkey_ = true;
}



/** ***************************************************************************/
void Core::GrabKeyButton::releaseAll() {
    releaseKeyboard();
    releaseMouse();
    waitingForHotkey_ = false;
}



/** ***************************************************************************/
void Core::GrabKeyButton::keyPressEvent(QKeyEvent *event) {
    if ( waitingForHotkey_ ) {
        // Modifier pressed -> update the label
        int key = event->key();
        int mods = event->modifiers();
        if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta ) {
            setText(QKeySequence((mods&~Qt::GroupSwitchModifier)|Qt::Key_Question).toString());//QTBUG-45568
            event->accept();
            return;
        }

        if(key == Qt::Key_Escape && mods == Qt::NoModifier) {
            event->accept();
            setText(oldText_);
            releaseAll(); // Can not be before since window closes on esc
            return;
        }
        releaseAll();

        setText(QKeySequence((mods&~Qt::GroupSwitchModifier)|key).toString()); //QTBUG-45568
        emit keyCombinationPressed(mods|key);
        return;
    }
//    QWidget::keyPressEvent( event );
}



/** ***************************************************************************/
void Core::GrabKeyButton::keyReleaseEvent(QKeyEvent *event) {
    if ( waitingForHotkey_ ) {
        // Modifier released -> update the label
        int key = event->key();
        if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
            setText(QKeySequence((event->modifiers()&~Qt::GroupSwitchModifier)|Qt::Key_Question).toString());//QTBUG-45568
            event->accept();
            return;
        }
        return;
    }
    QWidget::keyReleaseEvent( event );
}
