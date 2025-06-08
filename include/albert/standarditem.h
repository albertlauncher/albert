// SPDX-FileCopyrightText: 2024-2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/item.h>
#include <vector>

namespace albert::util
{

/// General purpose \ref Item implementation.
class ALBERT_EXPORT StandardItem : public Item
{
public:

    /// Constructs a \ref StandardItem with the contents initialized with the data passed.
    template<typename T1 = QString,
             typename T2 = QString,
             typename T3 = QString,
             typename T4 = QStringList,
             typename T5 = std::vector<Action>,
             typename T6 = QString>
        requires(std::same_as<std::remove_cvref_t<T1>, QString>
                 && std::same_as<std::remove_cvref_t<T2>, QString>
                 && std::same_as<std::remove_cvref_t<T3>, QString>
                 && std::same_as<std::remove_cvref_t<T4>, QStringList>
                 && std::same_as<std::remove_cvref_t<T5>, std::vector<Action>>
                 && std::same_as<std::remove_cvref_t<T6>, QString>)
    StandardItem(T1 &&id,
                 T2 &&text,
                 T3 &&subtext,
                 T4 &&icon_urls,
                 T5 &&actions = std::vector<Action>{},
                 T6 &&input_action_text = QString{}) noexcept :
        id_(std::forward<T1>(id)),
        text_(std::forward<T2>(text)),
        subtext_(std::forward<T3>(subtext)),
        icon_urls_(std::forward<T4>(icon_urls)),
        actions_(std::forward<T5>(actions)),
        input_action_text_(std::forward<T6>(input_action_text))
    {}

    /// Constructs a `shared_ptr` holding a \ref StandardItem with the contents initialized with the
    /// data passed.
    ///
    /// Convenience function for readability. See the \ref StandardItem::StandardItem for details.
    template<typename T1 = QString,
             typename T2 = QString,
             typename T3 = QString,
             typename T4 = QStringList,
             typename T5 = std::vector<Action>,
             typename T6 = QString>
        requires(std::same_as<std::decay_t<T1>, QString>
                 && std::same_as<std::decay_t<T2>, QString>
                 && std::same_as<std::decay_t<T3>, QString>
                 && std::same_as<std::decay_t<T4>, QStringList>
                 && std::same_as<std::decay_t<T5>, std::vector<Action>>
                 && std::same_as<std::decay_t<T6>, QString>)
    static std::shared_ptr<StandardItem> make(T1 &&id,
                                              T2 &&text,
                                              T3 &&subtext,
                                              T4 &&icon_urls,
                                              T5 &&actions = std::vector<Action>{},
                                              T6 &&input_action_text = QString{}) noexcept
    {
        return std::make_shared<StandardItem>(std::forward<T1>(id),
                                              std::forward<T2>(text),
                                              std::forward<T3>(subtext),
                                              std::forward<T4>(icon_urls),
                                              std::forward<T5>(actions),
                                              std::forward<T6>(input_action_text));
    }


    StandardItem(const StandardItem &) = delete;
    StandardItem& operator=(const StandardItem&) = delete;

    /// Constructs a \ref StandardItem with the contents of _other_ using move semantics.
    StandardItem(StandardItem &&other) noexcept = default;

    /// Replaces the contents with those of _other_ using move semantics.
    StandardItem &operator=(StandardItem &&other) noexcept = default;

    /// Sets the item identifier to _id_.
    void setId(QString id);

    /// Sets the item text to _text_.
    void setText(QString text);

    /// Sets the item subtext to _text_.
    void setSubtext(QString text);

    /// Sets the item icon urls to _urls_.
    void setIconUrls(QStringList urls);

    /// Sets the item actions to _actions_.
    void setActions(std::vector<Action> actions);

    /// Sets the item input action text to _text_.
    void setInputActionText(QString text);

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
    QStringList icon_urls_;
    std::vector<Action> actions_;
    QString input_action_text_;
};

}
