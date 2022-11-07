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
    Action(QString id, QString text, std::function<void()> function);

    QString id;  /// Identifier of the action
    QString text;  /// The description of the action
    std::function<void()> function;  /// Activates the item
};

class ALBERT_EXPORT Item  /// Items displayed in the query results list
{
public:
    virtual ~Item() = default;

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
    virtual QString inputActionText() const;

    /// Indicates that the item has actions
    virtual bool hasActions() const;

    /// The list of actions, this item provides
    virtual std::vector<Action> actions() const;

};

struct ALBERT_EXPORT IndexItem {
    IndexItem(std::shared_ptr<Item> item, QString string);
    std::shared_ptr<Item> item;
    QString string;
};

using Score = uint16_t;
const Score MAX_Score = std::numeric_limits<Score>::max();

struct ALBERT_EXPORT RankItem {
    RankItem(std::shared_ptr<Item> item, Score score);
    std::shared_ptr<Item> item;
    Score score;
};

}
