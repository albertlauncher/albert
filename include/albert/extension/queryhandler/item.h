// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/action.h"
#include <QStringList>
#include <vector>

namespace albert
{

/// Items displayed in the query results list
class ALBERT_EXPORT Item
{
public:
    virtual ~Item() = default;

    /// Getter for the item identifier.
    /// Has to be unique per extension.
    virtual QString id() const = 0;

    /// Getter for the item text.
    /// Primary text displayed emphasized in a list item. Empty text will
    /// crash the app since text length is used as divisor for scoring.
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
