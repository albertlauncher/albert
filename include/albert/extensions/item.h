// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../export.h"
#include <QStringList>
#include <functional>
#include <vector>

namespace albert
{

class ALBERT_EXPORT Action final  ///< Action for items
{
public:
    Action(QString id, QString text, std::function<void()> function);

    QString id;  ///< Identifier of the action
    QString text;  ///< Description of the action
    std::function<void()> function;  ///< The action function
};


class ALBERT_EXPORT Item  ///< Items displayed in the query results list
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
    virtual bool hasActions() const;  ///< Indicates that the item has actions
    virtual std::vector<Action> actions() const;  ///< List of item actions
};

}
