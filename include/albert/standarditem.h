// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/item.h>
#include <memory>
#include <vector>
namespace albert{ class Icon; }

namespace albert
{

///
/// General purpose \ref Item implementation.
///
/// \ingroup util_query
///
class ALBERT_EXPORT StandardItem : public Item
{
public:

    ///
    /// Constructs a \ref StandardItem with the contents initialized with the data passed.
    ///
    template<typename T_ID = QString,
             typename T_TEXT = QString,
             typename T_SUBTEXT = QString,
             typename T_ICON_FACTORY = std::function<std::unique_ptr<Icon>()>,
             typename T_ACTIONS = std::vector<Action>,
             typename T_INPUTACTION = QString>
        requires(std::same_as<std::remove_cvref_t<T_ID>, QString>
                 && std::same_as<std::remove_cvref_t<T_TEXT>, QString>
                 && std::same_as<std::remove_cvref_t<T_SUBTEXT>, QString>
                 && std::convertible_to<std::remove_cvref_t<T_ICON_FACTORY>, std::function<std::unique_ptr<Icon>()>>
                 && std::same_as<std::remove_cvref_t<T_ACTIONS>, std::vector<Action>>
                 && std::same_as<std::remove_cvref_t<T_INPUTACTION>, QString>)
    StandardItem(T_ID &&id,
                 T_TEXT &&text,
                 T_SUBTEXT &&subtext,
                 T_ICON_FACTORY &&icon_factory,
                 T_ACTIONS &&actions = std::vector<Action>{},
                 T_INPUTACTION &&input_action_text = QString{}) noexcept :
        id_(std::forward<T_ID>(id)),
        text_(std::forward<T_TEXT>(text)),
        subtext_(std::forward<T_SUBTEXT>(subtext)),
        icon_factory_(std::forward<T_ICON_FACTORY>(icon_factory)),
        actions_(std::forward<T_ACTIONS>(actions)),
        input_action_text_(std::forward<T_INPUTACTION>(input_action_text))
    {}


    ///
    /// Constructs a `shared_ptr` holding a \ref StandardItem with the contents initialized with the
    /// data passed.
    ///
    /// Convenience function for readability. See the \ref StandardItem::StandardItem for details.
    ///
    template<typename T_ID = QString,
             typename T_TEXT = QString,
             typename T_SUBTEXT = QString,
             typename T_ICON_FACTORY = std::function<std::unique_ptr<Icon>()>,
             typename T_ACTIONS = std::vector<Action>,
             typename T_INPUTACTION = QString>
        requires(std::same_as<std::decay_t<T_ID>, QString>
                 && std::same_as<std::decay_t<T_TEXT>, QString>
                 && std::same_as<std::decay_t<T_SUBTEXT>, QString>
                 && std::convertible_to<std::decay_t<T_ICON_FACTORY>, std::function<std::unique_ptr<Icon>()>>
                 && std::same_as<std::decay_t<T_ACTIONS>, std::vector<Action>>
                 && std::same_as<std::decay_t<T_INPUTACTION>, QString>)
    static std::shared_ptr<StandardItem> make(T_ID &&id,
                                              T_TEXT &&text,
                                              T_SUBTEXT &&subtext,
                                              T_ICON_FACTORY &&icon_factory,
                                              T_ACTIONS &&actions = std::vector<Action>{},
                                              T_INPUTACTION &&input_action_text = QString{}) noexcept
    {
        return std::make_shared<StandardItem>(std::forward<T_ID>(id),
                                              std::forward<T_TEXT>(text),
                                              std::forward<T_SUBTEXT>(subtext),
                                              std::forward<T_ICON_FACTORY>(icon_factory),
                                              std::forward<T_ACTIONS>(actions),
                                              std::forward<T_INPUTACTION>(input_action_text));
    }

    StandardItem(const StandardItem &) = delete;
    StandardItem& operator=(const StandardItem&) = delete;

    /// Constructs a \ref StandardItem with the contents of _other_ using move semantics.
    StandardItem(StandardItem &&other) noexcept = default;

    /// Replaces the contents with those of _other_ using move semantics.
    StandardItem &operator=(StandardItem &&other) noexcept = default;

    /// Destructs the \ref StandardItem.
    ~StandardItem();

    /// Sets the item identifier to _id_.
    void setId(QString id);

    /// Sets the item text to _text_.
    void setText(QString text);

    /// Sets the item subtext to _text_.
    void setSubtext(QString text);

    /// Sets the item factory to _icon_factory_.
    void setIconFactory(std::function<std::unique_ptr<Icon>()> icon_factory);

    /// Returns the item icon factory.
    std::function<std::unique_ptr<Icon>()> iconFactory();

    /// Sets the item actions to _actions_.
    void setActions(std::vector<Action> actions);

    /// Sets the item input action text to _text_.
    void setInputActionText(QString text);

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString inputActionText() const override;
    std::unique_ptr<Icon> icon() const override;
    std::vector<Action> actions() const override;

protected:
    QString id_;
    QString text_;
    QString subtext_;
    std::function<std::unique_ptr<Icon>()> icon_factory_;
    std::vector<Action> actions_;
    QString input_action_text_;
};

}
