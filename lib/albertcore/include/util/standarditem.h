// Copyright (C) 2014-2017 Manuel Schneider

#pragma once
#include <QString>
#include <vector>
#include <memory>
#include "core/item.h"

namespace Core {

class Action;

/**
* @brief A standard item
* If you dont need the flexibility subclassing the abstract classes provided,
* you can simply use this container, fill it with data.
*/
class EXPORT_CORE StandardItem : public Item
{
public:

    StandardItem(QString id = QString(),
                 QString iconPath = QString(),
                 QString text = QString(),
                 QString subtext = QString(),
                 QString completion = QString(),
                 Urgency urgency = Item::Urgency::Normal,
                 std::vector<std::shared_ptr<Action>> actions = std::vector<std::shared_ptr<Action>>())
        : id_(std::move(id)),
          iconPath_(std::move(iconPath)),
          text_(std::move(text)),
          subtext_(std::move(subtext)),
          completion_(std::move(completion)),
          urgency_(urgency),
          actions_(std::move(actions)) { }

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

protected:

    QString id_;
    QString iconPath_;
    QString text_;
    QString subtext_;
    QString completion_;
    Item::Urgency urgency_;
    std::vector<std::shared_ptr<Action>> actions_;

};

}
