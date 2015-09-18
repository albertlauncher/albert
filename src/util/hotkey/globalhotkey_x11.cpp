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

#include "globalhotkey_p.h"
#include "X11/Xutil.h"
#include "xcb/xcb.h"
#include <QtX11Extras/QX11Info>
#include <QVector>
#include <QSet>
#include <QAbstractEventDispatcher>

static u_int16_t _alt_mask;
static u_int16_t _meta_mask;
static u_int16_t _super_mask;
static u_int16_t _hyper_mask;
static u_int16_t _numlock_mask;
static bool failed;

/**************************************************************************/
struct Qt_XK_Keygroup {
		char num;
		int sym[3];
};

/**************************************************************************/
struct Qt_XK_Keymap {
		int key;
		Qt_XK_Keygroup xk;
};

/**************************************************************************/
struct GrabbedKey {
	uint mod;
	int code;
};
static QVector<GrabbedKey> _grabbedKeys;

/**************************************************************************/
static Qt_XK_Keymap Qt_XKSym_table[] = {
	{ Qt::Key_Escape,      {1, { XK_Escape }}},
	{ Qt::Key_F1,          {1, { XK_F1 }}},
	{ Qt::Key_F2,          {1, { XK_F2 }}},
	{ Qt::Key_F3,          {1, { XK_F3 }}},
	{ Qt::Key_F4,          {1, { XK_F4 }}},
	{ Qt::Key_F5,          {1, { XK_F5 }}},
	{ Qt::Key_F6,          {1, { XK_F6 }}},
	{ Qt::Key_F7,          {1, { XK_F7 }}},
	{ Qt::Key_F8,          {1, { XK_F8 }}},
	{ Qt::Key_F9,          {1, { XK_F9 }}},
	{ Qt::Key_F10,         {1, { XK_F10 }}},
	{ Qt::Key_F11,         {1, { XK_F11 }}},
	{ Qt::Key_F12,         {1, { XK_F12 }}},
	{ Qt::Key_Pause,       {1, { XK_Pause }}},
	{ Qt::Key_ScrollLock,  {1, { XK_Scroll_Lock }}},
	{ Qt::Key_Print,       {1, { XK_Print }}},
	{ Qt::Key_Insert,      {1, { XK_Insert, }}},
	{ Qt::Key_Delete,      {1, { XK_Delete }}},
	{ Qt::Key_Home,        {1, { XK_Home}}},
	{ Qt::Key_End,         {1, { XK_End }}},
	{ Qt::Key_PageUp,      {1, { XK_Prior }}},
	{ Qt::Key_PageDown,    {1, { XK_Next }}},
	{ Qt::Key_Left,        {1, { XK_Left }}},
	{ Qt::Key_Up,          {1, { XK_Up }}},
	{ Qt::Key_Right,       {1, { XK_Right }}},
	{ Qt::Key_Down,        {1, { XK_Down }}},
	{ Qt::Key_AsciiCircum, {1, { XK_dead_circumflex }}},
	{ Qt::Key_Tab,         {1, { XK_Tab }}},
	{ Qt::Key_CapsLock,    {1, { XK_Caps_Lock }}},
	{ Qt::Key_Shift,       {3, { XK_Shift_L, XK_Shift_R, XK_Shift_Lock  }}},
	{ Qt::Key_Control,     {2, { XK_Control_L, XK_Control_R }}},
	{ Qt::Key_Meta,        {2, { XK_Meta_L, XK_Meta_R }}},
	{ Qt::Key_Alt,         {2, { XK_Alt_L, XK_Alt_R }}},
	{ Qt::Key_Space,       {1, { XK_space }}},
	{ Qt::Key_Return,      {1, { XK_Return }}},
	{ Qt::Key_Asterisk,    {1, { XK_asterisk }}},
	{ Qt::Key_Backspace,   {1, { XK_BackSpace }}},
	{ Qt::Key_Enter,       {1, { XK_KP_Enter }}},
	{ Qt::Key_NumLock,     {1, { XK_Num_Lock }}},
	{ Qt::Key_Comma,       {1, { XK_comma }}},
	{ Qt::Key_Period,      {1, { XK_period }}},
	{ Qt::Key_Minus,       {1, { XK_minus }}},
	{ Qt::Key_Plus,        {1, { XK_plus }}},
	{ Qt::Key_unknown,     {0, { 0 }}},
};

/**************************************************************************/
static int XGrabErrorHandler(Display *, XErrorEvent *)
{
	failed = true;
	return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
GlobalHotkey::GlobalHotkeyPrivate::GlobalHotkeyPrivate(QObject *parent)
	: QObject(parent)
{
	// open Xlib stuff
	Display* dpy = QX11Info::display();
	if (dpy == NULL) {
		fprintf(stderr, "Error, unable to open X display: %s\n", XDisplayName(NULL));
		exit(EXIT_FAILURE);
	}

	// GET CORRECT MODIFIERS
	XModifierKeymap* map = XGetModifierMapping(dpy);
	if (map) {
		// XKeycodeToKeysym helper code adapeted from xmodmap
		int min_keycode, max_keycode, keysyms_per_keycode = 1;
		XDisplayKeycodes (dpy, &min_keycode, &max_keycode);
		XFree(XGetKeyboardMapping (dpy, min_keycode, (max_keycode - min_keycode + 1), &keysyms_per_keycode));

		int i, maskIndex = 0, mapIndex = 0;
		for (maskIndex = 0; maskIndex < 8; maskIndex++) {
			for (i = 0; i < map->max_keypermod; i++) {
				if (map->modifiermap[mapIndex]) {
					KeySym *sym = NULL;
                                        int keysym_return;
					int symIndex = 0;
					do {
                                                sym = XGetKeyboardMapping(dpy, map->modifiermap[mapIndex], symIndex, &keysym_return);
						symIndex++;
					} while ( !sym && symIndex < keysyms_per_keycode);
					if (_alt_mask == 0 && (*sym == XK_Alt_L || *sym == XK_Alt_R)) {
						_alt_mask = 1 << maskIndex;
					}
					if (_meta_mask == 0 && (*sym == XK_Meta_L || *sym == XK_Meta_R)) {
						_meta_mask = 1 << maskIndex;
					}
					if (_super_mask == 0 && (*sym == XK_Super_L || *sym == XK_Super_R)) {
						_super_mask = 1 << maskIndex;
					}
					if (_hyper_mask == 0 && (*sym == XK_Hyper_L || *sym == XK_Hyper_R)) {
						_hyper_mask = 1 << maskIndex;
					}
					if (_numlock_mask == 0 && (*sym == XK_Num_Lock)) {
						_numlock_mask = 1 << maskIndex;
					}
                                        XFree(sym);
				}
				mapIndex++;
			}
		}

		XFreeModifiermap(map);

		// logic from qt source see gui/kernel/qkeymapper_x11.cpp
		if (_meta_mask == 0 || _meta_mask == _alt_mask) {
			// no meta keys... s,meta,super,
			_meta_mask = _super_mask;
			if (_meta_mask == 0 || _meta_mask == _alt_mask) {
				// no super keys either? guess we'll use hyper then
				_meta_mask = _hyper_mask;
			}
		}
	}
	else {
		// assume defaults
		_alt_mask = Mod1Mask;
		_meta_mask = Mod4Mask;
	}

	QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

/***                    CODE BIN                     ***/
// TODO SCROLLOCK?

//	// Based on code from xbindkeys: grab_key.c (GPLv2)
//	int i;
//	XModifierKeymap *modmap;
//	KeyCode nlock, slock;
//	static int mask_table[8] = {
//		ShiftMask, LockMask, ControlMask, Mod1Mask,
//		Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
//	};
//	_lmasks.numlock = 0;
//	_lmasks.scrolllock = 0;
//	_lmasks.capslock = 0;

//	nlock = XKeysymToKeycode (_display, XK_Num_Lock);
//	slock = XKeysymToKeycode (_display, XK_Scroll_Lock);

//	modmap = XGetModifierMapping (_display);

//	if (modmap != NULL && modmap->max_keypermod > 0) {
//		for (i = 0; i < 8 * modmap->max_keypermod; i++) {
//			if (modmap->modifiermap[i] == nlock && nlock != 0)
//				_lmasks.numlock = mask_table[i / modmap->max_keypermod];
//			else if (modmap->modifiermap[i] == slock && slock != 0)
//				_lmasks.scrolllock = mask_table[i / modmap->max_keypermod];
//		}
//	}

//	_lmasks.capslock = LockMask;

//	if (modmap)
//		XFreeModifiermap (modmap);
}

/**************************************************************************/
bool GlobalHotkey::GlobalHotkeyPrivate::registerNativeHotkey(const int hk)
{
	int keyQt = hk & ~Qt::KeyboardModifierMask;
	int modQt = hk &  Qt::KeyboardModifierMask;


	/* Translate key symbol ( Qt -> X ) */

	Qt_XK_Keygroup kg;
	kg.num = 0;
	kg.sym[0] = 0;

	keyQt &= ~Qt::KeyboardModifierMask;

	bool found = false;
	for (int n = 0; Qt_XKSym_table[n].key != Qt::Key_unknown; ++n) {
			if (Qt_XKSym_table[n].key == keyQt) {
					kg = Qt_XKSym_table[n].xk;
					found = true;
					break;
			}
	}

	if (!found) {
			// try latin1
			if (keyQt >= 0x20 && keyQt <= 0x7f) {
					kg.num = 1;
					kg.sym[0] = keyQt;
			}
	}

	if (!kg.num)
			return false;


	/* Translate modifiers ( Qt -> X ) */

	unsigned int modsX = 0;
	if (modQt & Qt::META)
			modsX |= _meta_mask;
	if (modQt & Qt::SHIFT)
			modsX |= ShiftMask;
	if (modQt & Qt::CTRL)
			modsX |= ControlMask;
	if (modQt & Qt::ALT)
			modsX |= _alt_mask;



	// Prepare masks containing scrollock and caps combos too
	QSet<u_int16_t> masks;
	masks << (modsX | 0)
		  << (modsX | LockMask)
		  << (modsX | _numlock_mask)
		  << (modsX | LockMask | _numlock_mask );

	// Set own errorhandler
	XErrorHandler savedErrorHandler = XSetErrorHandler(XGrabErrorHandler);

	/* Grab the key combos (potenzmenge aus mods und keys) */
	failed = false;
	Display* dpy = QX11Info::display();
	Window rw = QX11Info::appRootWindow();
	for (int n = 0; !failed && n < kg.num; ++n) {
		KeyCode  XKCode = XKeysymToKeycode(QX11Info::display(), kg.sym[n]);
		// Apply key grab with prepared masks
		for (u_int16_t mask : masks) {
			XGrabKey(dpy, XKCode, mask, rw, true, GrabModeAsync, GrabModeAsync);
			if (!failed)
				_grabbedKeys << GrabbedKey({mask, XKCode});
		}
	}

	XSync(dpy, False);

	// Reset errorhandler
	XSetErrorHandler(savedErrorHandler);

	if (failed){
		// Unregister the partial registration
		unregisterNativeHotkeys();
		return false;
	}
	return true;
}

/**************************************************************************/
void GlobalHotkey::GlobalHotkeyPrivate::unregisterNativeHotkeys()
{
	for(const GrabbedKey &key : _grabbedKeys)
		XUngrabKey(QX11Info::display(), key.code, key.mod, QX11Info::appRootWindow());
	_grabbedKeys.clear();
}

/**************************************************************************/
bool GlobalHotkey::GlobalHotkeyPrivate::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
	Q_UNUSED(result);
	if (eventType == "xcb_generic_event_t") {
		xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
		if ((ev->response_type & 127) == XCB_KEY_PRESS)
		{
			xcb_key_press_event_t *k = (xcb_key_press_event_t *)ev;
			// Check if the key is one of the registered
			for (int i = 0; i < _grabbedKeys.size(); ++i)
				if (k->detail == _grabbedKeys[i].code && k->state == _grabbedKeys[i].mod)
				{
					emit hotKeyPressed();
					return true;
				}
		}
	}
	return false;
}
