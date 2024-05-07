// Copyright (c) 2021-2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include "albert/query/rankitem.h"
#include "indexitem.h"
#include <QString>
#include <vector>
#include <memory>

namespace albert
{

///
/// A fuzzy search index for items.
///
class ALBERT_EXPORT ItemIndex
{
public:
    ItemIndex(QString separators, bool case_sensitive, uint n, uint error_tolerance_divisor);
    ~ItemIndex();

    /// Set the items to be indexed.
    /// @param items The items to be indexed.
    void setItems(std::vector<IndexItem> &&items);

    /// Search the index for a string.
    /// @param string The string to search for.
    /// @param isValid A flag used to cancel the search.
    /// @return A list of scored items.
    std::vector<RankItem> search(const QString &string, const bool &isValid) const;

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
