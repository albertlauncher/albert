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
#include <functional>
#include "abstractitem.h"
using std::function;


/** ****************************************************************************
 * @brief A standard action
 * If you dont need the flexibility subclassing the abstract action provides,
 * you can simply use this container, fill it with data and put it into the
 * StandardItem.
 */
struct StandardAction final : public AbstractAction
{
public:

    StandardAction(){}
    StandardAction(const QString &text, function<void(ExecutionFlags *)> f)
        : text_(text), action_(f) {}

    QString text() const override { return text_; }
    void setText(const QString &text){text_ = text;}

    const function<void(ExecutionFlags *)>& action() { return action_; }
    void setAction(function<void(ExecutionFlags *)> action){ action_ = std::move(action);}

    void activate(ExecutionFlags *flags) override { action_(flags); }

private:

    QString text_;
    function<void(ExecutionFlags *)> action_;

};
typedef shared_ptr<StandardAction> SharedStdAction;
