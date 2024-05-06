// Copyright (c) 2021-2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
#include <vector>
#include <memory>
#include <memory>
#include <memory>
#include <memory>

namespace albert
{

class Item;
class RankItem;
class IndexItem;


/// An item utlized by ItemIndex
class ALBERT_EXPORT IndexItem
{
public:
    /// \param item \copydoc item
    /// \param string \copydoc string
    IndexItem(std::shared_ptr<Item> item, QString string);

    /// The item to be indexed
    std::shared_ptr<Item> item;

    /// The corresponding lookup string
    QString string;
};


class ALBERT_EXPORT ItemIndex
{
public:
    ItemIndex(QString separators, bool case_sensitive, uint n, uint error_tolerance_divisor);
    ~ItemIndex();
    void setItems(std::vector<albert::IndexItem> &&);
    std::vector<albert::RankItem> search(const QString &string, const bool &isValid) const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

}
