// Copyright (C) 2018-2019 Manuel Schneider

#include "standardindexitem.h"

Core::StandardIndexItem::StandardIndexItem(QString id, QString iconPath, QString text, QString subtext, QString completion, Core::Item::Urgency urgency, std::vector<std::shared_ptr<Core::Action> > actions, std::vector<Core::IndexableItem::IndexString> indexStrings)
    : id_(std::move(id)),
      iconPath_(std::move(iconPath)),
      text_(std::move(text)),
      subtext_(std::move(subtext)),
      completion_(std::move(completion)),
      urgency_(urgency),
      actions_(std::move(actions)),
      indexStrings_(std::move(indexStrings)) { }

QString Core::StandardIndexItem::id() const { return id_; }

void Core::StandardIndexItem::setId(QString id) { id_ = std::move(id); }

QString Core::StandardIndexItem::iconPath() const { return iconPath_; }

void Core::StandardIndexItem::setIconPath(QString iconPath) { iconPath_ = std::move(iconPath); }

QString Core::StandardIndexItem::text() const { return text_; }

void Core::StandardIndexItem::setText(QString text) { text_ = std::move(text); }

QString Core::StandardIndexItem::subtext() const { return subtext_; }

void Core::StandardIndexItem::setSubtext(QString subtext) { subtext_ = std::move(subtext); }

QString Core::StandardIndexItem::completion() const { return completion_; }

void Core::StandardIndexItem::setCompletion(QString completion) { completion_ = std::move(completion); }

Core::Item::Urgency Core::StandardIndexItem::urgency() const { return urgency_; }

void Core::StandardIndexItem::setUrgency(Core::Item::Urgency urgency) { urgency_ = urgency; }

std::vector<std::shared_ptr<Core::Action> > Core::StandardIndexItem::actions() { return actions_; }

void Core::StandardIndexItem::setActions(std::vector<std::shared_ptr<Core::Action> > actions) { actions_ = std::move(actions); }

void Core::StandardIndexItem::addAction(std::shared_ptr<Core::Action> action) { actions_.push_back(std::move(action)); }

std::vector<Core::IndexableItem::IndexString> Core::StandardIndexItem::indexStrings() const { return indexStrings_; }

void Core::StandardIndexItem::setIndexKeywords(std::vector<Core::IndexableItem::IndexString> indexStrings) { indexStrings_ = std::move(indexStrings); }
