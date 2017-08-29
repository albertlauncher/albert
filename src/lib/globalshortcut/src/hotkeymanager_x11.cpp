// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QKeySequence>
#include <map>
#include <set>
#include <QtX11Extras/QX11Info>
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/XKBlib.h"
#include "xcb/xcb.h"
#include "xcb/xproto.h"
#include "hotkeymanager_x11.h"

namespace {

    // http://cep.xray.aps.anl.gov/software/qt4-x11-4.2.2-browser/dc/d02/qkeymapper__x11_8cpp-source.html
    static std::map<uint, std::set<uint>> QtToXSymsMap =
    {
        { Qt::Key_Escape,       { XK_Escape }}, // misc keys
        { Qt::Key_Tab,          { XK_Tab }},
        //{ Qt::Key_Backtab,  { XK_}},
        { Qt::Key_Backspace,    { XK_BackSpace }},
        { Qt::Key_Return,       { XK_Return }},
        { Qt::Key_Enter,        { XK_KP_Enter }},
        { Qt::Key_Insert,       { XK_Insert, }},
        { Qt::Key_Delete,       { XK_Delete }},
        { Qt::Key_Pause,        { XK_Pause }},
        { Qt::Key_Print,        { XK_Print }},
        { Qt::Key_SysReq,       { XK_Sys_Req}},
        { Qt::Key_Clear,        { XK_Clear}},
        { Qt::Key_Home,         { XK_Home}},// cursor movement
        { Qt::Key_End,          { XK_End }},
        { Qt::Key_Left,         { XK_Left }},
        { Qt::Key_Up,           { XK_Up }},
        { Qt::Key_Right,        { XK_Right }},
        { Qt::Key_Down,         { XK_Down }},
        { Qt::Key_PageUp,       { XK_Prior }},
        { Qt::Key_PageDown,     { XK_Next }},
        { Qt::Key_Shift,        { XK_Shift_L, XK_Shift_R, XK_Shift_Lock  }},// modifiers
        { Qt::Key_Control,      { XK_Control_L, XK_Control_R }},
        { Qt::Key_Meta,         { XK_Meta_L, XK_Meta_R }},
        { Qt::Key_Alt,          { XK_Alt_L, XK_Alt_R }},
        { Qt::Key_CapsLock,     { XK_Caps_Lock }},
        { Qt::Key_NumLock,      { XK_Num_Lock }},
        { Qt::Key_ScrollLock,   { XK_Scroll_Lock }},
        { Qt::Key_F1,           { XK_F1 }},          // function keys
        { Qt::Key_F2,           { XK_F2 }},
        { Qt::Key_F3,           { XK_F3 }},
        { Qt::Key_F4,           { XK_F4 }},
        { Qt::Key_F5,           { XK_F5 }},
        { Qt::Key_F6,           { XK_F6 }},
        { Qt::Key_F7,           { XK_F7 }},
        { Qt::Key_F8,           { XK_F8 }},
        { Qt::Key_F9,           { XK_F9 }},
        { Qt::Key_F10,          { XK_F10 }},
        { Qt::Key_F11,          { XK_F11 }},
        { Qt::Key_F12,          { XK_F12 }},
        { Qt::Key_F13,          { XK_F13 }},
        { Qt::Key_F14,          { XK_F14 }},
        { Qt::Key_F15,          { XK_F15 }},
        { Qt::Key_F16,          { XK_F16 }},
        { Qt::Key_F17,          { XK_F17 }},
        { Qt::Key_F18,          { XK_F18 }},
        { Qt::Key_F19,          { XK_F19 }},
        { Qt::Key_F20,          { XK_F20 }},
        { Qt::Key_F21,          { XK_F21 }},
        { Qt::Key_F22,          { XK_F22 }},
        { Qt::Key_F23,          { XK_F23 }},
        { Qt::Key_F24,          { XK_F24 }},
        { Qt::Key_F25,          { XK_F25 }},  // F25 .. F35 only on X11
        { Qt::Key_F26,          { XK_F26 }},
        { Qt::Key_F27,          { XK_F27 }},
        { Qt::Key_F28,          { XK_F28 }},
        { Qt::Key_F29,          { XK_F29 }},
        { Qt::Key_F30,          { XK_F30 }},
        { Qt::Key_F31,          { XK_F31 }},
        { Qt::Key_F32,          { XK_F32 }},
        { Qt::Key_F33,          { XK_F33 }},
        { Qt::Key_F34,          { XK_F34 }},
        { Qt::Key_F35,          { XK_F35 }},
        { Qt::Key_Super_L,      { XK_Super_L }},      // extra keys
        { Qt::Key_Super_R,      { XK_Super_R }},
        { Qt::Key_Menu,         { XK_Menu }},
        { Qt::Key_Hyper_L,      { XK_Hyper_L }},
        { Qt::Key_Hyper_R,      { XK_Hyper_R }},
        { Qt::Key_Help,         { XK_Help }},
//        { Qt::Key_Direction_L,  { XK_}},
//        { Qt::Key_Direction_R,  { XK_}}
        /* ascii 0x20 to 0xff */
    };


    /** ***********************************************************************/
    bool lastGrabFailed;
    static int XGrabErrorHandler(Display *, XErrorEvent *e) {
        qWarning() << "XGrabError: "<< e->type;
        lastGrabFailed = true;
        return 0;
    }


//    /** ***********************************************************************/
//    class XLibManager
//    {
//    public:
//        XLibManager(){
//            display = XOpenDisplay(NULL); // Open $DISPLAY
//            if ( !display )
//                qFatal("Could not open DISPLAY");
//            defaultScreen = DefaultScreenOfDisplay(display);
//            defaultRootWindow = DefaultRootWindow(display);
//        }

//        ~XLibManager(){
//            XCloseDisplay(display);
//        }

//        Display *display;
//        Screen  *defaultScreen;
//        Window  defaultRootWindow;
//    };


//    /** ***********************************************************************/
//    class XCBManager
//    {
//    public:
//        XCBManager(){
//            connection = xcb_connect(NULL, &defaultScreenNumber); // Open $DISPLAY
//            defaultScreen = screen_of_display(connection, defaultScreenNumber);
//            if ( !defaultScreen )
//                qFatal("No default screen found!");
//            defaultRootWindow = defaultScreen->root;
//        }

//        ~XCBManager(){
//            xcb_disconnect(connection);
//        }

//        xcb_connection_t *connection;
//        int               defaultScreenNumber;
//        xcb_screen_t     *defaultScreen;
//        xcb_window_t      defaultRootWindow;
//    };



}

/** ***************************************************************************/
GlobalShortcut::HotkeyManagerPrivate::HotkeyManagerPrivate(QObject *parent)
    : QObject(parent) {

    Display* dpy = QX11Info::display();
//        xcb_connection_t* conn = QX11Info::connection();
//        xcb_generic_error_t **e = nullptr ;

    XModifierKeymap* map = XGetModifierMapping(dpy); // Contains the keys being used as modifiers.
    // TODO
//        xcb_get_modifier_mapping_cookie_t mmc = xcb_get_modifier_mapping(conn);
//        xcb_get_modifier_mapping_reply_t *mmr = xcb_get_modifier_mapping_reply (conn, mmc, e);

    masks = {0,0,0,0,0,0};
    if (map) {
        /* The XDisplayKeycodes() function returns the min-keycodes and
        max-keycodes supported by the specified display. The minimum number
        of KeyCodes returned is never less than 8, and the maximum number
        of KeyCodes returned is never greater than 255. Not all KeyCodes in
        this range are required to have corresponding keys.*/
        int min_keycode, max_keycode;
        XDisplayKeycodes (dpy, &min_keycode, &max_keycode);
//          min_keycode = xcb_get_setup(QX11Info::connection())->min_keycode;
//          max_keycode = xcb_get_setup(QX11Info::connection())->max_keycode;

        /* The XGetKeyboardMapping() function returns the symbols for the
        specified number of KeyCodes starting with first_keycode.
        KeySym number N, counting from zero, for KeyCode K has the following
        index in the list, counting from zero:
                (K - first_code) * keysyms_per_code_return + N
        A special KeySym value of NoSymbol is used to fill in unused
        elements for individual KeyCodes. To free the storage returned by
        XGetKeyboardMapping(), use XFree(). */
        int keysyms_per_keycode;
        XFree(XGetKeyboardMapping (dpy,
                                   (KeyCode)min_keycode,
                                   (max_keycode- min_keycode + 1),
                                   &keysyms_per_keycode));
//            uint8_t keysyms_per_keycode;
//            xcb_get_keyboard_mapping_cookie_t kmc =
//            xcb_get_keyboard_mapping_unchecked(X11Info::connection(),
//                                               (xcb_keycode_t)min_keycode,
//                                               (max_keycode- min_keycode + 1));
//            xcb_get_keyboard_mapping_reply_t *kmr =
//            xcb_get_keyboard_mapping_reply (X11Info::connection(),kmc,NULL);
//            keysyms_per_keycode = kmr->keysyms_per_keycode;
//            free(kmr);

        KeyCode kc;
        for (int maskIndex = 0; maskIndex < 8; maskIndex++) {
            for (int i = 0; i < map->max_keypermod; i++) {
                kc = map->modifiermap[maskIndex*map->max_keypermod+i];
                if (kc) {
                    KeySym sym;
                    int symIndex = 0;
                    do {
                        sym = XkbKeycodeToKeysym(dpy, kc, 0, symIndex);
//                            qDebug() <<  XKeysymToString(sym);
                        symIndex++;
                    } while ( sym == NoSymbol && symIndex < keysyms_per_keycode);
                    if (masks.alt == 0 && (sym == XK_Alt_L || sym == XK_Alt_R))
                        masks.alt = 1 << maskIndex;
                    if (masks.meta == 0 && (sym == XK_Meta_L || sym == XK_Meta_R))
                        masks.meta = 1 << maskIndex;
                    if (masks.super == 0 && (sym == XK_Super_L || sym == XK_Super_R))
                        masks.super = 1 << maskIndex;
                    if (masks.hyper == 0 && (sym == XK_Hyper_L || sym == XK_Hyper_R))
                        masks.hyper = 1 << maskIndex;
                    if (masks.numlock == 0 && (sym == XK_Num_Lock))
                        masks.numlock = 1 << maskIndex;
//                        if (masks.iso_level3_shift == 0 && (sym == XK_ISO_Level3_Shift))
//                            masks.iso_level3_shift = 1 << maskIndex;
                }
            }
        }
        XFreeModifiermap(map);

        // logic from qt source see gui/kernel/qkeymapper_x11.cpp
        if (masks.meta == 0 || masks.meta == masks.alt) {
            // no meta keys... s,meta,super,
            masks.meta = masks.super;
            if (masks.meta == 0 || masks.meta == masks.alt) {
                // no super keys either? guess we'll use hyper then
                masks.meta = masks.hyper;
            }
        }
    }
    else {
        // assume defaults
        masks.alt     = Mod1Mask;
        masks.meta    = Mod1Mask;
        masks.numlock = Mod2Mask;
        masks.super   = Mod4Mask;
        masks.hyper   = Mod4Mask;
//            masks.iso_level3_shift = Mod5Mask;
    }

    offendingMasks = {0, LockMask, masks.numlock, (LockMask|masks.numlock)};

    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
}



/** ***************************************************************************/
GlobalShortcut::HotkeyManagerPrivate::~HotkeyManagerPrivate() {

}



/** ***************************************************************************/
bool GlobalShortcut::HotkeyManagerPrivate::registerNativeHotkey(uint hotkey) {

    // Convert to native format
    std::set<uint> keysX = nativeKeycodes(hotkey & ~Qt::KeyboardModifierMask);
    uint modsX = nativeModifiers(hotkey &  Qt::KeyboardModifierMask);
    if ( keysX.empty() )
        return false;

    // Set own errorhandler
    XErrorHandler savedErrorHandler = XSetErrorHandler(XGrabErrorHandler);

    // Grab the key combos (potenzmenge aus mods und keys)
    lastGrabFailed = false;
    std::set<std::pair<uint,uint>> tmpGrabbedKeys;
    for ( uint keySym : keysX ) {

        // Break if a grab failed
        if ( lastGrabFailed )
            break;

        KeyCode XKCode = XKeysymToKeycode(QX11Info::display(), keySym);

        // On X lock and numlock are modifiers. Just grab all of them.
        for ( uint mask : offendingMasks ) {
            XGrabKey(QX11Info::display(), XKCode, modsX|mask, QX11Info::appRootWindow(), true, GrabModeAsync, GrabModeAsync);
            if ( !lastGrabFailed )
                tmpGrabbedKeys.insert({modsX|mask, XKCode});
        }
    }

    // Reset errorhandler
    XSetErrorHandler(savedErrorHandler);

    if ( !lastGrabFailed )
        std::copy(tmpGrabbedKeys.begin(), tmpGrabbedKeys.end(),
                  std::inserter(grabbedKeys, grabbedKeys.begin()));
    else
        // In case of error unregister the partial registration
        for (const std::pair<uint,uint> &p : tmpGrabbedKeys)
            XUngrabKey(QX11Info::display(), p.second, p.first, QX11Info::appRootWindow());

    XSync(QX11Info::display(), False);
    return !lastGrabFailed;



    //    xcb_void_cookie_t ck = xcb_grab_key(
    //                QX11Info::connection(),
    //                true,
    //                QX11Info::appRootWindow(),
    //                uint16_t modifiers,
    //                xcb_keycode_t key,
    //                XCB_GRAB_MODE_ASYNC,
    //                XCB_GRAB_MODE_ASYNC
    //                );


    //    xcb_generic_error_t *err = xcb_request_check (QX11Info::connection(), ck);
    //    if (err != NULL) {
    //        qWarning("X11 error %d", err->error_code);
    //        free (err);
    //    }
}



/** ***************************************************************************/
void GlobalShortcut::HotkeyManagerPrivate::unregisterNativeHotkey(uint hotkey) {

    // Convert to native format
    std::set<uint> keysX = nativeKeycodes(hotkey & ~Qt::KeyboardModifierMask);
    uint modsX = nativeModifiers(hotkey &  Qt::KeyboardModifierMask);
    if ( keysX.empty() )
        qCritical() << "keysX should not be empty";

    // Set own errorhandler
    XErrorHandler savedErrorHandler = XSetErrorHandler(XGrabErrorHandler);

    // UNgrab the key combos (potenzmenge aus mods und keys)
    for ( uint keySym : keysX ) {

        KeyCode XKCode = XKeysymToKeycode(QX11Info::display(), keySym);

        // On X lock and numlock are modifiers. Just ungrab all of them.
        for ( uint mask : offendingMasks ) {
            XUngrabKey(QX11Info::display(), XKCode, modsX|mask, QX11Info::appRootWindow());
            grabbedKeys.erase({modsX|mask, XKCode});
        }
    }

    // Reset errorhandler
    XSetErrorHandler(savedErrorHandler);

    XSync(QX11Info::display(), False);
}



/** ***************************************************************************/
std::set<uint> GlobalShortcut::HotkeyManagerPrivate::nativeKeycodes(uint qtKey) {
    /* Translate key symbol ( Qt -> X ) */
    // Use latin if possible
    if (qtKey >= 0x20 && qtKey <= 0xff)
        return std::set<uint>({qtKey});
    else {  // else check the handcrafted table for fancy keys
        auto it = QtToXSymsMap.find(qtKey);
        if ( it != QtToXSymsMap.end() )
            return it->second;
        else {
            qCritical() << "Could not translate key!"<< QKeySequence(qtKey).toString();
            return std::set<uint>();
        }
    }
}



/** ***************************************************************************/
uint GlobalShortcut::HotkeyManagerPrivate::nativeModifiers(uint qtMods) {
    uint ret = 0;
    //    if (qtMods & Qt::ShiftModifier)       ret |= XCB_MOD_MASK_SHIFT;
    //    if (qtMods & Qt::ControlModifier)     ret |= XCB_MOD_MASK_CONTROL;
    if (qtMods & Qt::ShiftModifier)       ret |= ShiftMask;
    if (qtMods & Qt::ControlModifier)     ret |= ControlMask;
    if (qtMods & Qt::AltModifier)         ret |= masks.alt;
    if (qtMods & Qt::MetaModifier)        ret |= masks.super;
//    if (qtMods & Qt::KeypadModifier)      ret |= masks.meta;
//    if (qtMods & Qt::GroupSwitchModifier) ret |= masks.iso_level3_shift;
    return ret;
}



/** ***************************************************************************/
bool GlobalShortcut::HotkeyManagerPrivate::nativeEventFilter(const QByteArray &eventType, void *message, long *) {
    if ( eventType == "xcb_generic_event_t" ) {
        xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
        if ( (ev->response_type & 127) == XCB_KEY_PRESS ) {
            xcb_key_press_event_t *k = reinterpret_cast<xcb_key_press_event_t*>(ev);
            // Check if the key is one of the registered
            for ( const std::pair<uint,uint> &p: grabbedKeys ) {
                if (k->detail == p.second && k->state == p.first) {
                    emit hotKeyPressed();
                    return true;
                }
            }
        }
    }
    return false;
}
