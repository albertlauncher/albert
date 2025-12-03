// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QStringList>
#include <albert/export.h>
#include <functional>
#include <vector>

namespace albert
{
class Icon;

///
/// Action used by result items (\ref Item).
///
/// \ingroup core_query
///
class ALBERT_EXPORT Action final
{
public:

    // ///
    // /// Constructs an \ref Action with the contents initialized with the data passed.
    // /// \param id_ \copybrief id
    // /// \param text_ \copybrief text
    // /// \param function_ \copybrief function
    // /// \param hide_on_activation_ \copybrief hide_on_activation
    // ///
    // template<typename T1 = QString,
    //          typename T2 = QString,
    //          typename T3 = std::function<void()>>
    // Action(T1 &&id_,
    //        T2 &&text_,
    //        T3 &&function_,
    //        bool hide_on_activation_ = true) noexcept :
    //     id(std::forward<T1>(id_)),
    //     text(std::forward<T2>(text_)),
    //     function(std::forward<T3>(function_)),
    //     hide_on_activation(hide_on_activation_)
    // {}

    ///
    /// The identifier.
    ///
    QString id;

    ///
    /// The description.
    ///
    QString text;

    ///
    /// The function executed on activation.
    ///
    std::function<void()> function;

    ///
    /// The activation behavior.
    ///
    bool hide_on_activation = true;
};


///
/// Result items displayed in the query results list
///
/// \ingroup core_query
///
class ALBERT_EXPORT Item
{
public:

    ///
    /// Destructs the item.
    ///
    virtual ~Item();

    ///
    /// Returns the item identifier.
    ///
    /// Has to be unique per extension. This function is involved in several time critical
    /// operartion such as indexing and sorting. It is therefore recommended to return a string that
    /// is as short as possible as fast as possible.
    ///
    virtual QString id() const = 0;

    ///
    /// Returns the item text.
    ///
    /// Primary text displayed emphasized in a list item. The string must not be empty, since the
    /// text length is used as divisor for scoring. Return as fast as possible. No checks are
    /// performed. If empty you get undefined behavior.
    ///
    virtual QString text() const = 0;

    ///
    /// Returns the item subtext.
    ///
    /// Secondary descriptive text displayed in a list item.x
    ///
    virtual QString subtext() const = 0;

    ///
    /// Returns the item input action text.
    ///
    /// Used as input text replacement (usually by pressing Tab).
    ///
    /// The base implementation returns \ref text().
    ///
    virtual QString inputActionText() const;

    ///
    /// Returns the item icon.
    ///
    /// Do _not_ clone a stored icon in this function. Icons can be a heavy resource.
    /// Instead return a new instance every time this function is called. The view will cache the icon instance.
    ///
    /// See \ref iconutil.h for the built-in icon factories.
    /// See \ref albert::Icon if you want to create your own icon engine.
    ///
    virtual std::unique_ptr<Icon> icon() const = 0;

    ///
    /// Returns the item actions.
    ///
    /// These are the actions a users can choose to activate. The base implementation returns an empty vector.
    ///
    virtual std::vector<Action> actions() const;

    ///
    /// Interface class for item observers
    ///
    class Observer
    {
    public:

        ///
        /// Notifies the observer about any changes in _item_.
        ///
        virtual void notify(const albert::Item *item) = 0;

    protected:

        ///
        /// Destructs the observer.
        ///
        virtual ~Observer();
    };

    ///
    /// Starts notifying _observer_ about any changes.
    ///
    virtual void addObserver(Observer *observer);

    ///
    /// Stops notifying _observer_ about any changes.
    ///
    virtual void removeObserver(Observer *observer);

};

namespace detail
{
class ALBERT_EXPORT DynamicItem : public Item
{
public:

    DynamicItem();
    ~DynamicItem() override;

    void dataChanged() const;

    void addObserver(Observer *) override;
    void removeObserver(Observer *) override;

private:

    class Private;
    std::unique_ptr<Private> d;

};
}

/// A shared pointer to an \ref Item or subclass thereof
template<typename T>
concept ItemPtr
    = std::is_base_of_v<Item, typename std::decay_t<T>::element_type>
      && std::same_as<std::shared_ptr<typename std::decay_t<T>::element_type>, std::decay_t<T>>;

/// A range of \ref ItemPtr
template<typename R>
concept ItemRange = std::ranges::range<R> && ItemPtr<std::ranges::range_value_t<R>>;

}  // namespace albert

