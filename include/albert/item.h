// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QStringList>
#include <albert/export.h>
#include <functional>
#include <vector>

namespace albert
{

/// Action used by result items (\ref Item).
class ALBERT_EXPORT Action final
{
public:

    /// Constructs an \ref Action with the contents initialized with the data passed.
    /// \param id \copybrief id
    /// \param text \copybrief text
    /// \param function \copybrief function
    /// \param hideOnActivation \copybrief hide_on_activation
    template<typename T1 = QString,
             typename T2 = QString,
             typename T3 = std::function<void()>>
    Action(T1 &&id_,
           T2 &&text_,
           T3 &&function_,
           bool hide_on_activation_ = true) noexcept :
        id(std::forward<T1>(id_)),
        text(std::forward<T2>(text_)),
        function(std::forward<T3>(function_)),
        hide_on_activation(hide_on_activation_)
    {}

    /// The identifier.
    QString id;

    /// The description.
    QString text;

    /// The function executed on activation.
    std::function<void()> function;

    /// The activation behavior.
    bool hide_on_activation;
};


///
/// Result items displayed in the query results list
///
class ALBERT_EXPORT Item
{
public:

    virtual ~Item();

    /// Returns the item identifier.
    /// Has to be unique per extension. This function is involved in several time critical
    /// operartion such as indexing and sorting. It is therefore recommended to return a string that
    /// is as short as possible as fast as possible.
    virtual QString id() const = 0;

    /// Returns the item text.
    /// Primary text displayed emphasized in a list item. This string is used in scoring. It is
    /// therefore recommended to return as fast as possible. The text length is used as divisor for
    /// scoring, hence the string must not be empty, otherwise you get undefined behavior. For
    /// performance reasons text length is not checked.
    virtual QString text() const = 0;

    /// Returns the item subtext.
    /// Secondary descriptive text displayed in a list item.
    virtual QString subtext() const = 0;

    /// Returns the items icon urls.
    /// Used to get the item icon using the icon provider functions.
    virtual QStringList iconUrls() const = 0;

    /// Returns the input action text.
    /// Used as input text replacement (usually by pressing Tab). The base implementation returns
    /// \ref text().
    virtual QString inputActionText() const;

    /// Returns the item actions.
    /// These are the actions a users can run. The base implementation returns an empty vector.
    virtual std::vector<Action> actions() const;

    /// Interface class for item observers
    class Observer
    {
    public:
        /// Notifies the observer about any changes in _item_.
        virtual void notify(const albert::Item *item) = 0;
    protected:
        virtual ~Observer();
    };

    /// Start notifying _observer_ about any changes.
    virtual void addObserver(Observer *observer);

    /// Stop notifying _observer_ about any changes.
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

}
