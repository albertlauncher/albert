// Copyright (C) 2014-2017 Manuel Schneider

#include "standardindexitem.h"
#include "action.h"
using std::vector;
using std::shared_ptr;
using namespace Core;


QString Core::StandardIndexItem::id() const { return id_; }
void Core::StandardIndexItem::setId(const QString &id) { id_ = id; }

QString Core::StandardIndexItem::text() const { return text_; }
void Core::StandardIndexItem::setText(const QString &text){ text_ = text; }

QString Core::StandardIndexItem::subtext() const { return subtext_; }
void Core::StandardIndexItem::setSubtext(const QString &subtext){ subtext_ = subtext; }

QString Core::StandardIndexItem::completionString() const { return completion_; }
void Core::StandardIndexItem::setCompletionString(const QString &completion) { completion_ = completion; }

QString Core::StandardIndexItem::iconPath() const { return iconPath_; }
void Core::StandardIndexItem::setIconPath(const QString &iconPath){ iconPath_ = iconPath; }

vector<Core::Action> Core::StandardIndexItem::actions(){ return actions_; }
void Core::StandardIndexItem::setActions(vector<Action> &&actions){ actions_ = std::move(actions); }

std::vector<IndexableItem::IndexString> StandardIndexItem::indexStrings() const {
    return indexStrings_;
}
void StandardIndexItem::setIndexKeywords(vector<IndexString> &&indexStrings) {
    indexStrings_ = indexStrings;
}
