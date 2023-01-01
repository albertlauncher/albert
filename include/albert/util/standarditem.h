// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../export.h"
#include "../extensions/queryhandler.h"
#include <memory>

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
            QString input_action_text = {},
            QStringList icon_urls = {},
            Actions actions = {});

    StandardItem(StandardItem&&) = default;
    StandardItem(const StandardItem&) = delete;
    StandardItem& operator=(StandardItem&&) = default;
    StandardItem& operator=(const StandardItem&) = delete;

    void setId(QString id);
    void setText(QString text);
    void setSubtext(QString subtext);
    void setInputActionText(QString input_action_text);
    void setIconUrls(QStringList icon_urls);
    void setActions(Actions actions);

    // albert::Item interface
    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString inputActionText() const override;
    QStringList iconUrls() const override;
    bool hasActions() const override;
    std::vector<Action> actions() const override;

    static std::shared_ptr<StandardItem> make(
            QString id = {},
            QString text = {},
            QString subtext = {},
            QString input_action_text = {},
            QStringList icon_urls = {},
            Actions actions = {});

    static std::shared_ptr<StandardItem> make(
            QString id = {},
            QString text = {},
            QString subtext = {},
            QStringList icon_urls = {},
            Actions actions = {});

protected:
    QString id_;
    QString text_;
    QString subtext_;
    QString input_action_text_;
    QStringList icon_urls_;
    Actions actions_;
};

}
