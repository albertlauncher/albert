// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include "standardindexitem.h"
#include "action.h"
using std::vector;
using std::shared_ptr;
using namespace Core;

Core::StandardIndexItem::StandardIndexItem(const QString &id) : id_(id) { }

QString Core::StandardIndexItem::id() const {
    return id_;
}

QString Core::StandardIndexItem::text() const {
    return text_;
}

void Core::StandardIndexItem::setText(const QString &text){
    text_ = text;
}

QString Core::StandardIndexItem::subtext() const {
    return subtext_;
}

void Core::StandardIndexItem::setSubtext(const QString &subtext){
    subtext_ = subtext;
}

QString Core::StandardIndexItem::completionString() const {
    return (completion_.isNull()) ? text_ : completion_;
}

void Core::StandardIndexItem::setCompletionString(const QString &completion) {
    completion_ = completion;
}

QString Core::StandardIndexItem::iconPath() const {
    return iconPath_;
}

void Core::StandardIndexItem::setIconPath(const QString &iconPath){
    iconPath_ = iconPath;
}

vector<shared_ptr<Core::Action>> Core::StandardIndexItem::actions(){
    return actions_;
}

void Core::StandardIndexItem::setActions(vector<shared_ptr<Action> > &&actions){
    actions_ = actions;
}

std::vector<IndexableItem::IndexString> StandardIndexItem::indexStrings() const {
    return indexStrings_;
}

void StandardIndexItem::setIndexKeywords(vector<IndexString> &&indexStrings) {
    indexStrings_ = indexStrings;
}
