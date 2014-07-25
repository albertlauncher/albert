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

#ifndef XHOTKEYMANAGER_H
#define XHOTKEYMANAGER_H

#include <QThread>
#include <QDebug>
#include <X11/Xlib.h>

class XHotKeyManager : public QThread
{
    Q_OBJECT

    typedef struct {
        unsigned int numlock;
        unsigned int capslock;
        unsigned int scrolllock;
    } xhkLockmasks;

public:
    static XHotKeyManager * getInstance(){
        if (instance == nullptr)
            instance = new XHotKeyManager();
        return instance;
    }
    void run() Q_DECL_OVERRIDE ;

signals:
    void hotKeyPressed();

private:
    XHotKeyManager();
    ~XHotKeyManager();

    static XHotKeyManager*  instance;
    Display*                _display;
    Window                  _root;
    xhkLockmasks            _lmasks;
    bool                    _continue;

    void grab_key(int keycode, unsigned int modifiers);
    void ungrab_key(int keycode, unsigned int modifiers);
    void getOffendingModifiers();
};

#endif // XHOTKEYMANAGER_H
