// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "item.h"

namespace albert {
struct ALBERT_EXPORT StandardItem : public Item
{
public:
    using ActionFactory = std::function<std::vector<Action>()>;

    explicit StandardItem(
            QString id = {},
            QString text = {},
            QString subtext = {},
            QStringList icon_urls = {},
            ActionFactory action_factory = {},
            QString input_action_text = {}
    ) : id_(id), text_(text), subtext_(subtext), icon_urls_(icon_urls),
        action_factory_(action_factory), input_action_text_(input_action_text) {}

    void setId(QString id) { id_ = id; }
    void setText(QString text) { text_ = std::move(text); }
    void setSubtext(QString subtext) { subtext_ = std::move(subtext); }
    void setIconUrls(QStringList icon_urls) { icon_urls_ = std::move(icon_urls); }
    void setActionFactory(ActionFactory action_factory) { action_factory_ = std::move(action_factory); }
    void setInputActionText(QString input_action_text) { input_action_text_ = std::move(input_action_text); }

    // albert::Item interface
    QString id() const override { return id_; }
    QString text() const override { return text_; }
    QString subtext() const override { return subtext_; }
    QStringList iconUrls() const override { return icon_urls_; }
    QString inputActionText() const override { return input_action_text_; }
    bool hasActions() const override { return action_factory_.target<std::vector<Action>()>() != nullptr; }
    std::vector<Action> actions() const override { return action_factory_(); }

protected:
    QString id_;
    QString text_;
    QString subtext_;
    QStringList icon_urls_;
    ActionFactory action_factory_;
    QString input_action_text_;

};
}
