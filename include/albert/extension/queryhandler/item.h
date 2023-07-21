// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "action.h"
#include <QStringList>
#include <vector>

namespace albert
{

/// Items displayed in the query results list
class ALBERT_EXPORT Item
{
public:
    virtual ~Item() = default;

    /// Per extension unique identifier.
    virtual QString id() const = 0;

    /// Primary text displayed.
    /// @note Empty will crash the app since text length is used as divisor for scoring
    virtual QString text() const = 0;

    /// The descriptive subtext displayed.
    virtual QString subtext() const = 0;

    /// The iconUrls to try for icon lookup.
    /// @see albert:IconProvider
    virtual QStringList iconUrls() const = 0;

    /// Input replacement for input action (usually Tab)
    virtual QString inputActionText() const;

    /// List of item actions
    virtual std::vector<Action> actions() const;
};

}
