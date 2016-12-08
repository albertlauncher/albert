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

#include "standardaction.h"

StandardAction::StandardAction(){

}

StandardAction::StandardAction(const QString &text, function<void ()> f)
    : text_(text), action_(f) {
}

QString StandardAction::text() const {
    return text_;
}

void StandardAction::setText(const QString &text){
    text_ = text;
}

const function<void ()> &StandardAction::action() {
    return action_;
}

void StandardAction::setAction(function<void ()> &&action){
    action_ = action;
}

void StandardAction::activate() {
    action_();
}
