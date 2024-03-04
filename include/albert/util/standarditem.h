// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/query/item.h"
#include <vector>

namespace albert
{

/// General purpose value type Item implememtation
class ALBERT_EXPORT StandardItem : public Item
{
public:
    /// \brief StandardItem constructor
    /// \param id @copydetails id
    /// \param text @copydetails text
    /// \param subtext @copydetails subtext
    /// \param input_action_text @copydetails iconUrls
    /// \param icon_urls @copydetails inputActionText
    /// \param actions @copydetails actions
    explicit StandardItem(
            QString id = {},
            QString text = {},
            QString subtext = {},
            QString input_action_text = {},
            QStringList icon_urls = {},
            std::vector<Action> actions = {});


    /// StandardItem move constructor
    StandardItem(StandardItem&&) = default;
    /// StandardItem move assignment
    StandardItem& operator=(StandardItem&&) = default;
    StandardItem(const StandardItem&) = delete;
    StandardItem& operator=(const StandardItem&) = delete;

    /// Setter for the item identifier @copydetails id
    void setId(QString id);

    /// Setter for the item text @copydetails text
    void setText(QString text);

    /// Setter for the item subtext @copydetails subtext
    void setSubtext(QString subtext);

    /// Setter for the items iconUrls @copydetails iconUrls
    void setInputActionText(QString input_action_text);

    /// Setter for the input action text @copydetails inputActionText
    void setIconUrls(QStringList icon_urls);

    /// Setter for item actions @copydetails actions
    void setActions(std::vector<Action> actions);

    // albert::Item interface
    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString inputActionText() const override;
    QStringList iconUrls() const override;
    std::vector<Action> actions() const override;

    /// Convenience shared pointer factory for standard items
    static std::shared_ptr<StandardItem> make(
            QString id = {},
            QString text = {},
            QString subtext = {},
            QString input_action_text = {},
            QStringList icon_urls = {},
            std::vector<Action> actions = {});

    /// Convenience shared pointer factory for standard items w/o inputAction
    static std::shared_ptr<StandardItem> make(
            QString id = {},
            QString text = {},
            QString subtext = {},
            QStringList icon_urls = {},
            std::vector<Action> actions = {});

protected:
    QString id_;
    QString text_;
    QString subtext_;
    QString input_action_text_;
    QStringList icon_urls_;
    std::vector<Action> actions_;
};

}
