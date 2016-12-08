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
#include "abstractitem.h"
#include "abstractaction.h"
#include "core_globals.h"
using std::vector;

/** ****************************************************************************
* @brief A standard item
* If you dont need the flexibility subclassing the abstract classes provided,
* you can simply use this container, fill it with data.
*/
class EXPORT_CORE StandardItem : public AbstractItem
{
public:

    StandardItem(const QString &id);

    QString id() const override final;

    QString text() const override final;
    void setText(const QString &text);

    QString subtext() const override final;
    void setSubtext(const QString &subtext);

    QString iconPath() const override final;
    void setIconPath( const QString &iconPath);

    vector<SharedAction> actions() override final;
    void setActions(vector<SharedAction> &&actions);

private:

    QString id_;
    QString text_;
    QString subtext_;
    QString iconPath_;
    vector<SharedAction> actions_;

};
typedef shared_ptr<StandardItem> SharedStdItem;
