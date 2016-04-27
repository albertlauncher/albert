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

#pragma once
#include <QString>
#include <memory>
#include <vector>
using std::shared_ptr;
using std::vector;



/** ****************************************************************************
 * @brief The AbstractItem
 * Displayable base class for all albert items.
 */
class Action
{
public:
    struct ExecutionFlags {
        // Mainwidget will be hidden after the action has been exectuted
        bool hideWidget = true;
        // Inputline will be cleared after the action has been exectuted
        bool clearInput = true;
    };

    virtual ~Action() {}

    /** A description */
    virtual QString text() const = 0;

    /** Activates the item */
    virtual void activate(ExecutionFlags *) = 0;
};
typedef shared_ptr<Action> ActionSPtr;
typedef vector<shared_ptr<Action>> ActionSPtrVec;


/** ****************************************************************************
 * @brief The A(bstract)A(lbert)Item - A3Item for convenience
 * This the most prominent class here. This is the base class for all you want -
 * let your items be visible in the proposal list. Subclass this item to your
 * liking and add it to the query if you think it matches the query context.
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
    virtual QString subtext() const = 0;

    /** The usage count used for the ranking in the list */
    virtual uint16_t usageCount() const { return 0; }

    /** Urgency level of the item, defautls to "Normal" */
    virtual Urgency urgency() const { return Urgency::Normal; }

    /** The alternative actions of the item*/
    virtual ActionSPtrVec actions() { return ActionSPtrVec(); }

    /** The children of the item */
    virtual AlbertItem* parent() { return nullptr; }

    /** The children of the item */
    virtual vector<shared_ptr<AlbertItem>> children() { return vector<shared_ptr<AlbertItem>>(); }

    /** Indicates if the item has children, for performance  */
    virtual bool hasChildren() const { return false; }
};
typedef shared_ptr<AlbertItem> ItemSPtr;
typedef vector<shared_ptr<AlbertItem>> ItemSPtrVec;

