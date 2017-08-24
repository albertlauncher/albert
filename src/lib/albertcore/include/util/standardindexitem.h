// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

    StandardIndexItem(const QString &id = QString());

    QString id() const override final;

    QString text() const override;
    void setText(const QString &text);

    QString subtext() const override;
    void setSubtext(const QString &subtext);

    QString completionString() const override;
    void setCompletionString(const QString &completion);

    QString iconPath() const override;
    void setIconPath( const QString &iconPath);

    std::vector<std::shared_ptr<Action>> actions() override;
    void setActions(std::vector<std::shared_ptr<Action>> &&actions);

    virtual std::vector<Core::IndexableItem::WeightedKeyword> indexKeywords() const override;
    virtual void setIndexKeywords(std::vector<IndexableItem::WeightedKeyword> &&indexKeywords);

private:

    QString id_;
    QString text_;
    QString subtext_;
    QString completion_;
    QString iconPath_;
    std::vector<std::shared_ptr<Action>> actions_;
    std::vector<IndexableItem::WeightedKeyword> indexKeywords_;

};

}
