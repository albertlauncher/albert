// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QString>
#include <vector>
#include <memory>
#include "albert/item.h"

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
                 std::vector<std::shared_ptr<Action>> actions = std::vector<std::shared_ptr<Action>>());

    QString id() const override;
    void setId(QString id);

    QString iconPath() const override;
    void setIconPath(QString iconPath);

    QString text() const override;
    void setText(QString text);

    QString subtext() const override;
    void setSubtext(QString subtext);

    QString completion() const override;
    void setCompletion(QString completion);

    Item::Urgency urgency() const override;
    void setUrgency(Item::Urgency urgency);

    std::vector<std::shared_ptr<Action>> actions() override;
    void setActions(std::vector<std::shared_ptr<Action>> &&actions);
    void setActions(const std::vector<std::shared_ptr<Action>> &actions);
    void addAction(std::shared_ptr<Action> &&action);
    void addAction(const std::shared_ptr<Action> &action);

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
