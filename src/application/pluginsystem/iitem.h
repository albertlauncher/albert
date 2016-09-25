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
using std::vector;
using std::shared_ptr;

#include "action.h"

/** ****************************************************************************
 * @brief The item interface
 * Subclass this class to make your object displayable in the results list.
 */
class AlbertItem : public Action
{
public:

    /** An enumeration of urgency levels */
    enum class Urgency : unsigned char {
        Low, // Use this if your extension tends to produce much potentially less relevant items (e.g. files)
        Normal, // In all other cases use this
        Notification, // Use this if your items are few, seldom and have to be on top (e.g. calculator output)
        Alert // Like notification. Gets a visual emphasis
    };

    virtual ~AlbertItem() {}

    /** The icon for the item */
    virtual QString iconPath() const = 0;

    /** The declarative subtext for the item */
    virtual QString subtext(const QString& query) const = 0;

    /** The usage count used for the ranking in the list */
    virtual uint16_t usageCount() const { return 0; }

    /** Urgency level of the item, defautls to "Normal" */
    virtual Urgency urgency() const { return Urgency::Normal; }

    /** The alternative actions of the item*/
    virtual ActionSPtrVec actions() { return ActionSPtrVec(); }

};
typedef shared_ptr<AlbertItem> ItemSPtr;
typedef vector<shared_ptr<AlbertItem>> ItemSPtrVec;

