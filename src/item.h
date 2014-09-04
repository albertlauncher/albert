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

#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <unistd.h>

#include "abstractlauncher.h"

class Item {
	Item() = delete;
	explicit Item(int id, AbstractLauncher *owner)
		: _internalId(id), _owner(owner){}

	const int _internalId;
	AbstractLauncher * const _owner;

	inline QString name() const { return _owner->name(_internalId); }
	inline QIcon icon() const { return _owner->icon(_internalId); }
	inline void primaryAction() const { _owner->primaryAction(); }
	inline void secondaryAction() const { _owner->secondaryAction(); }
	inline QString primaryActionText() const { return _owner->primaryActionText(_internalId); }
	inline QString secondaryActionText() const { return _owner->secondaryActionText(); }
};

#endif # ITEM_H
