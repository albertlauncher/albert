// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/item.h>
#include <vector>

namespace albert::util
{

/// General purpose value type Item implememtation
class ALBERT_EXPORT StandardItem : public Item
{
public:
    /// Constructs a StandardItem with the contents initialized with the data passed.
    /// \param id @copybrief id
    /// \param text @copybrief text
    /// \param subtext @copybrief subtext
    /// \param input_action_text @copybrief inputActionText
    /// \param icon_urls @copybrief iconUrls
    /// \param actions @copybrief actions
    explicit StandardItem(QString id,
                          QString text = {},
                          QString subtext = {},
                          QString input_action_text = {},
                          QStringList icon_urls = {},
                          std::vector<Action> actions = {}) noexcept;

    StandardItem(const StandardItem &) = delete;
    StandardItem& operator=(const StandardItem&) = delete;

    /// Constructs a StandardItem with the contents of `other` using move semantics.
    StandardItem(StandardItem &&other) noexcept = default;

    /// Replaces the contents with those of `other` using move semantics.
    StandardItem &operator=(StandardItem &&other) noexcept = default;

    /// Sets the item identifier to `id`.
    void setId(QString id);

    /// Sets the item text to `text`.
    void setText(QString text);

    /// Sets the item subtext to `text`.
    void setSubtext(QString text);

    /// Sets the item input action text to `text`.
    void setInputActionText(QString text);

    /// Sets the item icon urls to `urls`.
    void setIconUrls(QStringList urls);

    /// Sets the item actions to `actions`.
    void setActions(std::vector<Action> actions);

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

    // albert::Item interface
    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString inputActionText() const override;
    QStringList iconUrls() const override;
    std::vector<Action> actions() const override;

protected:
    QString id_;
    QString text_;
    QString subtext_;
    QString input_action_text_;
    QStringList icon_urls_;
    std::vector<Action> actions_;
};

}
