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

#include "standardobjects.h"

/******************************************************************************/

StandardAction::StandardAction(){}

StandardAction::StandardAction(const QString &text, function<void (ExecutionFlags *)> f)
    : text_(text), action_(f) {
}

QString StandardAction::text() const {
    return text_;
}

void StandardAction::setText(const QString &text){
    text_ = text;
}

const function<void (ExecutionFlags *)> &StandardAction::action() {
    return action_;
}

void StandardAction::setAction(function<void (ExecutionFlags *)> &&action){
    action_ = action;
}

void StandardAction::activate(ExecutionFlags *flags) {
    action_(flags);
}

/******************************************************************************/

StandardItem::StandardItem(const QString &id) : id_(id) {

}

QString StandardItem::id() const {
    return id_;
}

QString StandardItem::text() const {
    return text_;
}

void StandardItem::setText(const QString &text){
    text_ = text;
}

QString StandardItem::subtext() const {
    return subtext_;
}

void StandardItem::setSubtext(const QString &subtext){
    subtext_ = subtext;
}

QString StandardItem::iconPath() const {
    return iconPath_;
}

void StandardItem::setIconPath(const QString &iconPath){
    iconPath_ = iconPath;
}

vector<SharedAction> StandardItem::actions(){
    return actions_;
}

void StandardItem::setActions(vector<SharedAction> &&actions){
    actions_ = actions;
}

/******************************************************************************/

StandardIndexItem::StandardIndexItem(const QString &id)
    : StandardItem(id) {
}

vector<IIndexable::WeightedKeyword> StandardIndexItem::indexKeywords() const {
    return indexKeywords_;
}

void StandardIndexItem::setIndexKeywords(vector<WeightedKeyword> &&indexKeywords) {
    indexKeywords_ = indexKeywords;
}
