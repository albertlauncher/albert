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

#include "xhotkeymanager.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>

XHotKeyManager * XHotKeyManager::instance = nullptr;

/**************************************************************************//**
 * @brief XHotKeyManager::XHotKeyManager
 */
XHotKeyManager::XHotKeyManager() : _display(XOpenDisplay(NULL))
{
    // open Xlib stuff
    _display = XOpenDisplay(NULL);
    if (_display == NULL) {
        fprintf(stderr, "Error, unable to open X display: %s\n", XDisplayName(NULL));
        exit(EXIT_FAILURE);
    }
    _root = DefaultRootWindow(_display);
    getOffendingModifiers();

    _continue = true;

	grab_key(XK_space, Mod1Mask);// ControlMask

    XSelectInput(_display, _root, KeyPressMask );
}

/**************************************************************************//**
 * @brief XHotKeyManager::~XHotKeyManager
 */
XHotKeyManager::~XHotKeyManager()
{
    ungrab_key(XK_space, ControlMask);
    XCloseDisplay(_display);
    _display = NULL;
}

/**************************************************************************//**
 * @brief XHotKeyManager::run
 */
void XHotKeyManager::run()
{
    XEvent ev;
    bool hkPressed = false;
    while(_continue)
    {
        XNextEvent(_display, &ev);  //BLOCKS
        switch(ev.type)
        {
		case KeyPress:
            hkPressed = true;
            break;
        case KeyRelease:
			if (hkPressed){
				emit hotKeyPressed();
                hkPressed= false;
            }
            break;
        default:
            std::cout << "Defalt!" << std::endl;
            break;
        }
    }
}

/**************************************************************************//**
 * @brief XHotKeyManager::grab_key
 * @param keycode
 * @param modifiers
 */
void XHotKeyManager::grab_key(int keycode, unsigned int modifiers)
{
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers,
             _root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers | _lmasks.scrolllock,
             _root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers | _lmasks.capslock ,
             _root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers | _lmasks.numlock,
             _root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers | _lmasks.capslock | _lmasks.scrolllock,
             _root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers | _lmasks.numlock | _lmasks.scrolllock,
             _root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers | _lmasks.numlock | _lmasks.capslock,
             _root, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(_display, XKeysymToKeycode(_display,keycode),
             modifiers | _lmasks.numlock | _lmasks.capslock | _lmasks.scrolllock,
             _root, False, GrabModeAsync, GrabModeAsync);
}

/**************************************************************************//**
 * @brief XHotKeyManager::ungrab_key
 * @param keycode
 * @param modifiers
 */
void XHotKeyManager::ungrab_key(int keycode, unsigned int modifiers)
{
    XUngrabKey (_display, keycode, modifiers, _root);
    XUngrabKey (_display, keycode, modifiers | _lmasks.numlock, _root);
    XUngrabKey (_display, keycode, modifiers | _lmasks.capslock, _root);
    XUngrabKey (_display, keycode, modifiers | _lmasks.scrolllock, _root);
    XUngrabKey (_display, keycode, modifiers | _lmasks.capslock | _lmasks.scrolllock, _root);
    XUngrabKey (_display, keycode, modifiers | _lmasks.numlock | _lmasks.scrolllock, _root);
    XUngrabKey (_display, keycode, modifiers | _lmasks.numlock | _lmasks.capslock, _root);
    XUngrabKey (_display, keycode, modifiers | _lmasks.numlock | _lmasks.capslock | _lmasks.scrolllock, _root);
}

/**************************************************************************//**
 * @brief XHotKeyManager::getOffendingModifiers
 * @return
 */
void XHotKeyManager::getOffendingModifiers ()
{
    // Based on code from xbindkeys: grab_key.c (GPLv2)
    int i;
    XModifierKeymap *modmap;
    KeyCode nlock, slock;
    static int mask_table[8] = {
        ShiftMask, LockMask, ControlMask, Mod1Mask,
        Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };
    _lmasks.numlock = 0;
    _lmasks.scrolllock = 0;
    _lmasks.capslock = 0;

    nlock = XKeysymToKeycode (_display, XK_Num_Lock);
    slock = XKeysymToKeycode (_display, XK_Scroll_Lock);

    modmap = XGetModifierMapping (_display);

    if (modmap != NULL && modmap->max_keypermod > 0) {
        for (i = 0; i < 8 * modmap->max_keypermod; i++) {
            if (modmap->modifiermap[i] == nlock && nlock != 0)
                _lmasks.numlock = mask_table[i / modmap->max_keypermod];
            else if (modmap->modifiermap[i] == slock && slock != 0)
                _lmasks.scrolllock = mask_table[i / modmap->max_keypermod];
        }
    }

    _lmasks.capslock = LockMask;

    if (modmap)
        XFreeModifiermap (modmap);
}




























///**************************************************************************//**
// * @brief XHotKeyManager::registerHotKey
// * @return
// */
//bool XHotKeyManager::registerHotKey()
//{

//}

///**************************************************************************//**
// * @brief XHotKeyManager::unregisterHotKey
// * @return
// */
//bool XHotKeyManager::unregisterHotKey()
//{

//}
