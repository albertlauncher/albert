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
#include <memory>
using std::shared_ptr;


/** ****************************************************************************
 * @brief The AbstractItem
 * Displayable base class for all albert items.
 * Determines the bahaviour of the app after the action has been executed
 */
struct ExecutionFlags {
    /** Mainwidget will be hidden */
    bool hideWidget = true;
    /** Inputline will be cleared */
    bool clearInput = true;
};



/** ****************************************************************************
 * @brief The action interface
 * A base class for actions (and items)
 */
class AbstractAction
{
public:

    virtual ~AbstractAction() {}

    /** A description */
    virtual QString text() const = 0;

    /** Activates the item */
    virtual void activate(ExecutionFlags *) = 0;
};
typedef shared_ptr<AbstractAction> SharedAction;
