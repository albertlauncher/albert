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

#include "standarditem.h"
#include "action.h"

Core::StandardItem::StandardItem(const QString &id) : id_(id) { }

QString Core::StandardItem::id() const {
    return id_;
}

QString Core::StandardItem::text() const {
    return text_;
}

void Core::StandardItem::setText(const QString &text){
    text_ = text;
}

QString Core::StandardItem::subtext() const {
    return subtext_;
}

void Core::StandardItem::setSubtext(const QString &subtext){
    subtext_ = subtext;
}

QString Core::StandardItem::iconPath() const {
    return iconPath_;
}

void Core::StandardItem::setIconPath(const QString &iconPath){
    iconPath_ = iconPath;
}

vector<shared_ptr<Core::Action>> Core::StandardItem::actions(){
    return actions_;
}

void Core::StandardItem::setActions(vector<shared_ptr<Action> > &&actions){
    actions_ = actions;
}
