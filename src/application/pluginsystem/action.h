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
#include <vector>

/** ****************************************************************************
 * @brief The action interface
 * A base class for actions (and items)
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
    virtual QString text(const QString& query) const = 0;

    /** Activates the item */
    virtual void activate(ExecutionFlags *, const QString& query) = 0;
};
typedef std::shared_ptr<Action> ActionSPtr;
typedef std::vector<std::shared_ptr<Action>> ActionSPtrVec;
