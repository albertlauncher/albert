// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "item.h"

namespace albert
{

using Actions = std::vector<Action>;

struct ALBERT_EXPORT StandardItem : public Item
{
public:
    explicit StandardItem(
            QString id = {},
            QString text = {},
            QString subtext = {},
            QStringList icon_urls = {},
            Actions actions = {},
            QString input_action_text = {}):
    id_(std::move(id)),
    text_(std::move(text)),
    subtext_(std::move(subtext)),
    icon_urls_(std::move(std::move(icon_urls))),
    actions_(std::move(actions)),
    input_action_text_(std::move(input_action_text)) {}

    void setId(QString id) { id_ = std::move(id); }
    void setText(QString text) { text_ = std::move(text); }
    void setSubtext(QString subtext) { subtext_ = std::move(subtext); }
    void setInputActionText(QString input_action_text) { input_action_text_ = std::move(input_action_text); }
    void setIconUrls(QStringList icon_urls) { icon_urls_ = std::move(icon_urls); }
    void setActions(Actions actions) { actions_ = std::move(actions); }

    // albert::Item interface
    QString id() const override { return id_; }
    QString text() const override { return text_; }
    QString subtext() const override { return subtext_; }
    QString inputActionText() const override { return input_action_text_; }
    QStringList iconUrls() const override { return icon_urls_; }
    bool hasActions() const override { return !actions_.empty(); }
    std::vector<Action> actions() const override { return actions_; }

protected:
    QString id_;
    QString text_;
    QString subtext_;
    QStringList icon_urls_;
    Actions actions_;
    QString input_action_text_;
};

}
