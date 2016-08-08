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
* @brief A standard item
* If you dont need the flexibility subclassing the abstract albert item
* provides, you can simply use this container, fill it with data and put it into
* the query.
*/
class StandardItem final : public AbstractItem
{
public:

    StandardItem(const QString &id) : id_(id) {}
    StandardItem(const QString &id, const QString &text, const QString &subtext,
                 const QString &iconPath, vector<SharedAction> actions)
        : id_(id), text_(text), subtext_(subtext), iconPath_(iconPath), actions_(actions) {}

    QString id() const override { return id_; }

    QString text() const override { return text_; }
    void setText(const QString &text){text_ = text;}

    QString subtext() const override { return subtext_; }
    void setSubtext(const QString &subtext){subtext_ = subtext;}

    QString iconPath() const override { return iconPath_; }
    void setIconPath( const QString &iconPath){iconPath_ = iconPath;}

    vector<SharedAction> actions() { return actions_; }
    void setActions(vector<SharedAction> actions){ actions_ = std::move(actions);}

    void activate(ExecutionFlags *flags) override {
        if (!actions_.empty()) actions_[0]->activate(flags);
    }

private:

    QString id_;
    QString text_;
    QString subtext_;
    QString iconPath_;
    vector<SharedAction> actions_;

};
typedef shared_ptr<StandardItem> SharedStdItem;
