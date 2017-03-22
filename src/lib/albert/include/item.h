// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#pragma once
#include <QString>
#include <vector>
#include <memory>
#include "core_globals.h"

namespace Core {

class Action;

/** ****************************************************************************
 * @brief The item interface
 * Subclass this class to make your object displayable in the results list.
 */
class EXPORT_CORE Item
{

public:

    /**
     * An enumeration of urgency levels
     * Notifications are placed on top. Alert too but additionally get an visual
     * emphasis. Normal items are not handled in a special way.
     */
    enum class Urgency : unsigned char { Normal, Notification, Alert };

    virtual ~Item() {}

    /** An persistant, extensionwide unique identifier, "" if item is dynamic */
    virtual QString id() const = 0;

    /** The icon for the item */
    virtual QString iconPath() const = 0;

    /** The title for the item */
    virtual QString text() const = 0;

    /** The declarative subtext for the item */
    virtual QString subtext() const = 0;

    /** The string to use for completion */
    virtual QString completionString() const { return text(); }

    /** Urgency level of the item, defautls to "Normal" */
    virtual Urgency urgency() const { return Urgency::Normal; }

    /** The alternative actions of the item*/
    virtual std::vector<std::shared_ptr<Action>> actions() = 0;

};

}
