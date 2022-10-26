// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

namespace albert
{

struct ALBERT_EXPORT Action
{
    const QString id;
    const QString text;
    const std::function<void()> function;
};

/// Interface class for result items, displayed in the query results list
class ALBERT_EXPORT Item
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
    virtual QString inputActionText() const = 0;

    /// Indicates that the item has actions
    virtual bool hasActions() const { return false; }

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

}
