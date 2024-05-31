// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/indexitem.h>
#include <albert/rankitem.h>
#include <memory>
#include <vector>

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
