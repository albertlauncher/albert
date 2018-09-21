// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <vector>
#include "standarditem.h"
#include "core/indexable.h"

namespace Core {

/**
* @brief A standard index item for use with the offline index
* If you dont need the flexibility subclassing the abstract classes provided,
* you can simply use this container, fill it with data.
*/
class EXPORT_CORE StandardIndexItem final : public IndexableItem
{
public:

    StandardIndexItem(QString id = QString(),
                 QString iconPath = QString(),
                 QString text = QString(),
                 QString subtext = QString(),
                 QString completion = QString(),
                 Urgency urgency = Item::Urgency::Normal,
                 std::vector<std::shared_ptr<Action>> actions = std::vector<std::shared_ptr<Action>>(),
                 std::vector<Core::IndexableItem::IndexString> indexStrings = std::vector<Core::IndexableItem::IndexString>())
        : id_(std::move(id)),
          iconPath_(std::move(iconPath)),
          text_(std::move(text)),
          subtext_(std::move(subtext)),
          completion_(std::move(completion)),
          urgency_(urgency),
          actions_(std::move(actions)),
          indexStrings_(std::move(indexStrings)) { }

    QString id() const override { return id_; }
    void setId(QString id) { id_ = std::move(id); }

    QString iconPath() const override { return iconPath_; }
    void setIconPath(QString iconPath) { iconPath_ = std::move(iconPath); }

    QString text() const override { return text_; }
    void setText(QString text) { text_ = std::move(text); }

    QString subtext() const override { return subtext_; }
    void setSubtext(QString subtext) { subtext_ = std::move(subtext); }

    QString completion() const override { return completion_; }
    void setCompletion(QString completion) { completion_ = std::move(completion); }

    Item::Urgency urgency() const override { return urgency_; }
    void setUrgency(Item::Urgency urgency) { urgency_ = urgency; }

    std::vector<std::shared_ptr<Action>> actions() override { return actions_; }
    void setActions(std::vector<std::shared_ptr<Action>> actions) { actions_ = std::move(actions); }
    void addAction(std::shared_ptr<Action> action) { actions_.push_back(std::move(action)); }

    virtual std::vector<Core::IndexableItem::IndexString> indexStrings() const override { return indexStrings_; }
    void setIndexKeywords(std::vector<Core::IndexableItem::IndexString> indexStrings) { indexStrings_ = std::move(indexStrings); }


protected:

    QString id_;
    QString iconPath_;
    QString text_;
    QString subtext_;
    QString completion_;
    Item::Urgency urgency_;
    std::vector<std::shared_ptr<Action>> actions_;
    std::vector<IndexableItem::IndexString> indexStrings_;

};

}
