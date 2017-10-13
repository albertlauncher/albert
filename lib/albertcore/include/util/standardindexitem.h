// Copyright (C) 2014-2017 Manuel Schneider

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

    template<class S1 = QString,
             class S2 = QString,
             class S3 = QString,
             class S4 = QString,
             class S5 = QString,
             class S6 = Item::Urgency,
             class S7 = std::vector<std::shared_ptr<Action>>,
             class S8 = std::vector<IndexableItem::IndexString>>
    StandardIndexItem(S1&& id = QString(),
                      S2&& iconPath = QString(),
                      S3&& text = QString(),
                      S4&& subtext = QString(),
                      S5&& completion = QString(),
                      S6&& urgency = Item::Urgency::Normal,
                      S7&& actions = std::vector<std::shared_ptr<Action>>(),
                      S8&& indexstrings = std::vector<IndexableItem::IndexString>())
        : id_(std::forward<S1>(id)),
          iconPath_(std::forward<S2>(iconPath)),
          text_(std::forward<S3>(text)),
          subtext_(std::forward<S4>(subtext)),
          completion_(std::forward<S5>(completion)),
          urgency_(std::forward<S6>(urgency)),
          actions_(std::forward<S7>(actions)),
          indexStrings_(std::forward<S8>(indexstrings)) { }

    QString id() const override { return id_; }
    template<class T>
    void setId(T&& id) { id_ = std::forward<T>(id); }

    template<class T>
    void setIconPath(T&& iconPath) { iconPath_ = std::forward<T>(iconPath); }
    QString iconPath() const override { return iconPath_; }

    QString text() const override { return text_; }
    template<class T>
    void setText(T&& text) { text_ = std::forward<T>(text); }

    QString subtext() const override { return subtext_; }
    template<class T>
    void setSubtext(T&& subtext) { subtext_ = std::forward<T>(subtext); }

    QString completion() const override { return completion_; }
    template<class T>
    void setCompletion(T&& completion) { completion_ = std::forward<T>(completion); }

    Item::Urgency urgency() const override { return urgency_; }
    template<class T>
    void setUrgency(T&& urgency) { iconPath_ = std::forward<T>(urgency); }

    std::vector<std::shared_ptr<Action>> actions() override{ return actions_; }
    template<class T>
    void setActions(T&& actions) { actions_ = std::forward<T>(actions); }
    template<class T>
    void addAction(T&& action) { actions_.push_back(std::forward<T>(action)); }
    template<class T>
    void addActions(T&& action) { actions_.push_back(std::forward<T>(action)); }
    template<typename T, typename... Args>
    void addActions(T &&action, Args... remainder) {
        actions_.push_back(std::forward<T>(action));
        addActions(std::forward<T>(remainder)...) ;
    }

    virtual std::vector<Core::IndexableItem::IndexString> indexStrings() const override { return indexStrings_; }
    template<class T>
    void setIndexKeywords(T &&indexStrings) { indexStrings_ = std::forward<T>(indexStrings); }


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
