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
    /// URLs supported by the icon provider to create the icons displayed
    /// @note Icons are cached using the item id as key
    /// 'xdg:<icon-name>' performs freedesktop icon theme specification lookup (linux only).
    /// 'qfip:<path>' uses QFileIconProvider to get the icon for thefile.
    /// ':<path>' is a QResource path.
    /// '<path>' is interpreted as path to a local image file.
    virtual QStringList iconUrls() const = 0;
    virtual QString inputActionText() const;  ///< Input replacement for input action (usually Tab)
    virtual std::vector<Action> actions() const;  ///< List of item actions
};

}
