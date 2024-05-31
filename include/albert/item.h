// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QStringList>
#include <albert/action.h>
#include <albert/export.h>
#include <vector>

namespace albert
{

///
/// Result items displayed in the query results list
///
class ALBERT_EXPORT Item
{
public:

    virtual ~Item();

    /// Getter for the item identifier.
    ///
    /// Has to be unique per extension.
    ///
    /// This function is involved in several time critical operartion such as
    /// indexing and sorting. It is therefore recommended to return a string
    /// that is as short as possible as fast as possible.
    virtual QString id() const = 0;

    /// Getter for the item text.
    ///
    /// Primary text displayed emphasized in a list item.
    ///
    /// This string is used in scoring. It is therefore recommended to return
    /// as fast as possible. The text length is used as divisor for scoring,
    /// hence the string must not be empty, otherwise you get undefined
    /// behavior. For performance reasons text length is not checked.
    virtual QString text() const = 0;

    /// Getter for the item subtext.
    /// Secondary descriptive text displayed in a list item.
    virtual QString subtext() const = 0;

    /// Getter for the items iconUrls.
    /// Used to get the item icon using the IconProvider.
    virtual QStringList iconUrls() const = 0;

    /// Getter for the input action text.
    /// Used as input text replacement (usually by pressing Tab).
    virtual QString inputActionText() const;

    /// Getter for item actions.
    /// These are the actions a users can run.
    virtual std::vector<Action> actions() const;

};

}
