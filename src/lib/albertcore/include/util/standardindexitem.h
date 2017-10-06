// Copyright (C) 2014-2017 Manuel Schneider

#pragma once
#include <vector>
#include "standarditem.h"
#include "core/indexable.h"

namespace Core {

/** ****************************************************************************
* @brief A standard index item
* If you dont need the flexibility subclassing the abstract classes provided,
* you can simply use this container, fill it with data.
*/
class EXPORT_CORE StandardIndexItem final : public IndexableItem
{
public:

    StandardIndexItem(const QString &id = QString(),
                      const QString &text = QString(),
                      const QString &subtext = QString(),
                      const QString &completion = QString(),
                      const QString &iconPath = QString(),
                      std::vector<Action> &&actions = std::vector<Action>(),
                      std::vector<IndexString> &&indexString = std::vector<IndexString>())
        : id_(id),
          text_(text),
          subtext_(subtext),
          completion_(completion),
          iconPath_(iconPath),
          actions_(std::move(actions)) ,
          indexStrings_(std::move(indexString)) { }

    QString id() const override final;
    void setId(const QString &id);

    QString text() const override;
    void setText(const QString &text);

    QString subtext() const override;
    void setSubtext(const QString &subtext);

    QString completionString() const override;
    void setCompletionString(const QString &completion);

    QString iconPath() const override;
    void setIconPath(const QString &iconPath);

    std::vector<Action> actions() override;
    void setActions(std::vector<Action> &&actions);

    virtual std::vector<IndexString> indexStrings() const override;
    virtual void setIndexKeywords(std::vector<IndexString> &&indexStrings);

private:

    QString id_;
    QString text_;
    QString subtext_;
    QString completion_;
    QString iconPath_;
    std::vector<Action> actions_;
    std::vector<IndexableItem::IndexString> indexStrings_;

};

}
