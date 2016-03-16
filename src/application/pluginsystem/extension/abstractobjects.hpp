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
class Action{
public:
    virtual ~Action() {}
    virtual QIcon icon() const = 0;
    virtual QString text() const = 0;
    virtual QString subtext() const = 0;
    virtual void activate() = 0;
};



/** ****************************************************************************
 * @brief The A(bstract)A(lbert)Item - A3Item for convenience
 * This the most prominent class here. This is the base class for all you want -
 * let your items be visible in the proposal list. Subclass this item to your
 * liking and add it to the query if you think it matches the query context.
 */
class AlbertItem : public Action {
public:
    virtual std::vector<QString> aliases() const {return {text()};}
    virtual uint16_t usageCount() const {return 0;}
    virtual uint8_t importance() const {return 0;}

    virtual ~AlbertItem() {}
    virtual bool hasActions() const {return false;}
    virtual vector<shared_ptr<Action>> actions() {return vector<shared_ptr<Action>>();}
    virtual bool hasChildren() const {return false;}
    virtual vector<shared_ptr<AlbertItem>> children() {return vector<shared_ptr<AlbertItem>>();}
};
