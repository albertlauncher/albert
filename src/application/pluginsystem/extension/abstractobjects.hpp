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
#include <QIcon>
#include <memory>
#include <vector>
using std::shared_ptr;
using std::vector;



/** ****************************************************************************
 * @brief The AbstractItem
 * Displayable base class for all albert items.
 */
struct AlbertAction {
    virtual ~AlbertAction() {}

    /**
     * @brief Access to the data to display.
     *
     * Used to supply item data to views and delegates. Generally, models only
     * need to supply data for Qt::DisplayRole and any application-specific
     * user roles, but it is also good practice to provide data for
     * Qt::ToolTipRole, Qt::AccessibleTextRole, and
     * Qt::AccessibleDescriptionRole. See the Qt::ItemDataRole enum
     * documentation for information about the types associated with each role.
     * http://doc.qt.io/qt-5/qt.html#ItemDataRole-enum
     *
     * @param role The types of requested data
     * @return The requested data
     */
    virtual QString text() const = 0;
    virtual QString subtext() const = 0;
    virtual vector<QString> aliases() const { return {text()}; }
    virtual QIcon icon() const = 0;

    /**
     * @brief Exectute the Action.
     * Reimplement this if you want your item to do some actions on activation
     */
    virtual void activate() = 0;
};



/** ****************************************************************************
 * @brief The A(bstract)A(lbert)Item - A3Item for convenience
 * This the most prominent class here. This is the base class for all you want -
 * let your items be visible in the proposal list. Subclass this item to your
 * liking and add it to the query if you think it matches the query context.
 */
struct AlbertItem : public AlbertAction {
    virtual ~AlbertItem() {}

    /**
     * @brief Component accessor
     * Return the i-th compontent. Currently albert supports only two layers in
     * the view. The toplevel items and their children, the actions.
     * @return The i-th component.
     */

    virtual bool hasChildren() const {
        return false;
    }

    virtual vector<shared_ptr<AlbertItem>> children() {
        return vector<shared_ptr<AlbertItem>>();
    }
};
