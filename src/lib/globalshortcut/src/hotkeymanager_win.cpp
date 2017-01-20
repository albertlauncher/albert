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

#include "hotkeymanager_win.h"
#include "windows.h"

namespace {

static int sid = 0;
QSet<int> sGrabbedIds;

struct Qt_VK_Keymap
{
    int key;
    UINT vk;
};

static Qt_VK_Keymap Qt_VK_table[] = { // TODO make this hold groups too e.g. META r+l
        { Qt::Key_Escape,      VK_ESCAPE },
        { Qt::Key_Tab,         VK_TAB },
        { Qt::Key_Backtab,     0 },
        { Qt::Key_Backspace,   VK_BACK },
        { Qt::Key_Return,      VK_RETURN },
        { Qt::Key_Enter,       VK_RETURN },
        { Qt::Key_Insert,      VK_INSERT },
        { Qt::Key_Delete,      VK_DELETE },
        { Qt::Key_Pause,       VK_PAUSE },
        { Qt::Key_Print,       VK_SNAPSHOT },
        { Qt::Key_SysReq,      0 },
        { Qt::Key_Clear,       VK_CLEAR },
        { Qt::Key_Home,        VK_HOME },
        { Qt::Key_End,         VK_END },
        { Qt::Key_Left,        VK_LEFT },
        { Qt::Key_Up,          VK_UP },
        { Qt::Key_Right,       VK_RIGHT },
        { Qt::Key_Down,        VK_DOWN },
        { Qt::Key_PageUp,      VK_PRIOR },
        { Qt::Key_PageDown,    VK_NEXT },
        { Qt::Key_Shift,       VK_SHIFT },
        { Qt::Key_Control,     VK_CONTROL },
        { Qt::Key_Meta,        VK_LWIN },
        { Qt::Key_Alt,         VK_MENU },
        { Qt::Key_CapsLock,    VK_CAPITAL },
        { Qt::Key_NumLock,     VK_NUMLOCK },
        { Qt::Key_ScrollLock,  VK_SCROLL },
        { Qt::Key_F1,          VK_F1 },
        { Qt::Key_F2,          VK_F2 },
        { Qt::Key_F3,          VK_F3 },
        { Qt::Key_F4,          VK_F4 },
        { Qt::Key_F5,          VK_F5 },
        { Qt::Key_F6,          VK_F6 },
        { Qt::Key_F7,          VK_F7 },
        { Qt::Key_F8,          VK_F8 },
        { Qt::Key_F9,          VK_F9 },
        { Qt::Key_F10,         VK_F10 },
        { Qt::Key_F11,         VK_F11 },
        { Qt::Key_F12,         VK_F12 },
        { Qt::Key_F13,         VK_F13 },
        { Qt::Key_F14,         VK_F14 },
        { Qt::Key_F15,         VK_F15 },
        { Qt::Key_F16,         VK_F16 },
        { Qt::Key_F17,         VK_F17 },
        { Qt::Key_F18,         VK_F18 },
        { Qt::Key_F19,         VK_F19 },
        { Qt::Key_F20,         VK_F20 },
        { Qt::Key_F21,         VK_F21 },
        { Qt::Key_F22,         VK_F22 },
        { Qt::Key_F23,         VK_F23 },
        { Qt::Key_F24,         VK_F24 },
        { Qt::Key_F25,         0 },
        { Qt::Key_F26,         0 },
        { Qt::Key_F27,         0 },
        { Qt::Key_F28,         0 },
        { Qt::Key_F29,         0 },
        { Qt::Key_F30,         0 },
        { Qt::Key_F31,         0 },
        { Qt::Key_F32,         0 },
        { Qt::Key_F33,         0 },
        { Qt::Key_F34,         0 },
        { Qt::Key_F35,         0 },
        { Qt::Key_Super_L,     0 },
        { Qt::Key_Super_R,     0 },
        { Qt::Key_Menu,        0 },
        { Qt::Key_Hyper_L,     0 },
        { Qt::Key_Hyper_R,     0 },
        { Qt::Key_Help,        0 },
        { Qt::Key_Direction_L, 0 },
        { Qt::Key_Direction_R, 0 },

        { Qt::Key_QuoteLeft,   VK_OEM_8 },
        { Qt::Key_Minus,       VK_OEM_MINUS },
        { Qt::Key_Equal,       VK_OEM_PLUS },

        { Qt::Key_BracketLeft, VK_OEM_4 },
        { Qt::Key_BracketRight,VK_OEM_6 },

        { Qt::Key_Semicolon,   VK_OEM_1 },
        { Qt::Key_Apostrophe,  VK_OEM_3 },
        { Qt::Key_NumberSign,  VK_OEM_7 },

        { Qt::Key_Backslash,   VK_OEM_5 },
        { Qt::Key_Comma,	   VK_OEM_COMMA },
        { Qt::Key_Period,      VK_OEM_PERIOD },
        { Qt::Key_Slash,       VK_OEM_2 },

        { Qt::Key_unknown,     0 },
};

}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
HotkeyManagerPrivate::HotkeyManagerPrivate(QObject *parent)
    : QObject(parent)
{

    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
}

/**************************************************************************/
bool HotkeyManagerPrivate::registerNativeHotkey(const int hk)
{
    int keyQt = hk & ~Qt::KeyboardModifierMask;
    int modQt = hk &  Qt::KeyboardModifierMask;


    /* Translate key symbol ( Qt -> X ) */
    UINT key = 0;
    if (keyQt == 0x20 || //SPACE
            (keyQt >= 0x30 && keyQt <= 0x39) || // NUMBRS
            (keyQt > 0x41 && keyQt <= 0x5a) || // LETTERS
            (keyQt > 0x60 && keyQt <= 0x7a))   //NUMPAD
        key = keyQt;
    else {
        // Others require lookup from a keymap
        for (int n = 0; Qt_VK_table[n].key != Qt::Key_unknown; ++n) {
            if (Qt_VK_table[n].key == keyQt) {
                key = Qt_VK_table[n].vk;
                break;
            }
        }
        if (!key)
            return false;
    }


    /* Translate modifiers ( Qt -> X ) */

    unsigned int mods = 0; // MOD_NOREPEAT;
    if (modQt & Qt::META)
            mods |= MOD_WIN;
    if (modQt & Qt::SHIFT)
            mods |= MOD_SHIFT;
    if (modQt & Qt::CTRL)
            mods |= MOD_CONTROL;
    if (modQt & Qt::ALT)
            mods |= MOD_ALT;

    /* Grab the key combo*/
    bool success;
    success = RegisterHotKey(NULL, sid, mods, key);
    if (success)
        sGrabbedIds.insert(sid++);
    return success;
}

/**************************************************************************/
void HotkeyManagerPrivate::unregisterNativeHotkeys()
{
    for ( int i : sGrabbedIds)
        UnregisterHotKey(NULL, i);
}

/**************************************************************************/
bool HotkeyManagerPrivate::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY)
        {
            // Check if the key is one of the registered
            for (int i : sGrabbedIds)
                if (msg->wParam == i)
                {
                    emit hotKeyPressed();
                    return true;
                }
        }
    }
    return false;
}
