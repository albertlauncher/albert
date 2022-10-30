// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

namespace albert
{

struct ALBERT_EXPORT Action  /// A base class for actions (and items)
{
    const QString id;  /// Identifier of the action
    const QString text;  /// The description of the action
    const std::function<void()> function;  /// Activates the item
};


class ALBERT_EXPORT Item  /// Items displayed in the query results list
{
public:
    virtual ~Item() {}

    /// Persistent, extension-wide unique identifier, used for MRU sorting
    virtual QString id() const = 0;

    /// The title for the item
    virtual QString text() const = 0;

    /// The descriptive subtext
    virtual QString subtext() const = 0;

    /// URLs for the icon provider
    /// xdg:<icon-name> performs freedesktop icon theme specification lookup
    /// (linux only). qfip:<path> uses QFileIconProvider to get the icon for
    /// thefile. Otherwise the string is interpreted as path to an image file,
    /// whereby ':<path>' is a QResource path.
    /// @note Icons are cached.
    virtual QStringList iconUrls() const = 0;

    /// Used to replace the input when the input action is triggered (Tab)
    virtual QString inputActionText() const { return {}; };

    /// Indicates that the item has actions
    virtual bool hasActions() const { return true; }

    /// The list of actions, this item provides
    virtual std::vector<Action> actions() const { return {}; }
//
//    /// Indicates that the item has childrens
//    virtual bool hasChildren() const { return false; }
//
//    /// The list of actions, this item provides
//    virtual std::vector<SharedItem> children() const { return {}; }

};

using SharedItem = std::shared_ptr<Item>;
using SharedItemVector = std::vector<SharedItem>;
using Score = uint16_t;

struct ALBERT_EXPORT Match
{
    SharedItem item;
    Score score;
};

}
