// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
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

    virtual QString id() const = 0;  ///< Per extension unique identifier
    virtual QString text() const = 0;  ///< Primary text displayed
    virtual QString subtext() const = 0;  ///< The descriptive subtext displayed
    virtual QStringList iconUrls() const = 0;  ///< The iconUrls to try for icon lookup. @see albert:IconProvider
    virtual QString inputActionText() const;  ///< Input replacement for input action (usually Tab)
    virtual std::vector<Action> actions() const;  ///< List of item actions
};

}
