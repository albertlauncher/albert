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
    StandardItem() {}

    template<class S>
    StandardItem(S&& id)
        : id_(std::forward<S>(id)) { }

    template<class S1, class S2>
    StandardItem(S1&& id, S2&& text)
        : id_(std::forward<S1>(id)),
          text_(std::forward<S2>(text)) { }

    template<class S1, class S2, class S3>
    StandardItem(S1&& id, S2&& text, S3&& subtext)
        : id_(std::forward<S1>(id)),
          text_(std::forward<S2>(text)),
          subtext_(std::forward<S3>(subtext)) { }

    template<class S1, class S2, class S3, class S4>
    StandardItem(S1&& id, S2&& text, S3&& subtext, S4&& completion)
        : id_(std::forward<S1>(id)),
          text_(std::forward<S2>(text)),
          subtext_(std::forward<S3>(subtext)),
          completion_(std::forward<S4>(completion)) { }

    template<class S1, class S2, class S3, class S4, class S5>
    StandardItem(S1&& id, S2&& text, S3&& subtext, S4&& completion, S5&& iconPath)
        : id_(std::forward<S1>(id)),
          text_(std::forward<S2>(text)),
          subtext_(std::forward<S3>(subtext)),
          completion_(std::forward<S4>(completion)),
          iconPath_(std::forward<S5>(iconPath)) { }

    template<class S1, class S2, class S3, class S4, class S5, class S6>
    StandardItem(S1&& id, S2&& text, S3&& subtext, S4&& completion, S5&& iconPath, S6&& actions)
        : id_(std::forward<S1>(id)),
          text_(std::forward<S2>(text)),
          subtext_(std::forward<S3>(subtext)),
          completion_(std::forward<S4>(completion)),
          iconPath_(std::forward<S5>(iconPath)),
          actions_(std::forward<S6>(actions)) { }

    QString id() const override { return id_; }
    template<class T> void setId(T&& id) { id_ = std::forward<T>(id); }

    QString text() const override { return text_; }
    template<class T> void setText(T&& text) { text_ = std::forward<T>(text); }

    QString subtext() const override { return subtext_; }
    template<class T> void setSubtext(T&& subtext) { subtext_ = std::forward<T>(subtext); }

    QString completionString() const override { return completion_; }
    template<class T> void setCompletionString(T&& completion) { completion_ = std::forward<T>(completion); }

    QString iconPath() const override { return iconPath_; }
    template<class T> void setIconPath(T&& iconPath) { iconPath_ = std::forward<T>(iconPath); }

    std::vector<Action> actions() override{ return actions_; }
    void setActions(const std::vector<Action>& actions) { actions_ = actions; }
    void setActions(std::vector<Action>&& actions) { actions_ = std::move(actions); }
    void addAction(const Action& action) { actions_.push_back(action); }
    void addAction(Action&& action) { actions_.push_back(std::move(action)); }
    template<class T1, class T2>
    void emplaceAction(T1&& text, T2 function) {
        actions_.emplace_back(std::forward<T1>(text), std::forward<T2>(function));
    }

protected:

    QString id_;
    QString text_;
    QString subtext_;
    QString completion_;
    QString iconPath_;
    std::vector<Action> actions_;

};

}
