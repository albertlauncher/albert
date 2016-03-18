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
#include <functional>
#include "abstractobjects.hpp"
using std::function;

class StandardItem final : public AlbertItem
{
public:
    StandardItem(){}
    StandardItem(const QString &text, const QString &subtext, const QString &iconPath, std::function<void()> f)
        : text_(text), subtext_(subtext), iconPath_(iconPath), action_(f) {}

    QString text() const override { return text_; }
    void setText(const QString &text){text_ = text;}

    QString subtext() const override { return subtext_; }
    void setSubtext(const QString &subtext){subtext_ = subtext;}

    QString iconPath() const override { return iconPath_; }
    void setIcon( const QString &iconPath){iconPath_ = iconPath;}

    std::function<void()> action() { return action_; }
    void setAction(std::function<void()> action){ action_ = std::move(action);}
    void activate() override { action_();}

private:
    QString text_;
    QString subtext_;
    QString iconPath_;
    function<void()> action_;
};

