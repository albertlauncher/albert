// SPDX-FileCopyrightText: 2024 Manuel Schneider

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/indexitem.h>
#include <albert/matchconfig.h>
#include <albert/rankitem.h>
#include <memory>
#include <vector>

///
/// A fuzzy search index for items.
///
class ALBERT_EXPORT ItemIndex final
{
public:

    ItemIndex(albert::MatchConfig config = {});
    ItemIndex(ItemIndex &&);
    ItemIndex& operator=(ItemIndex &&);
    ~ItemIndex();

    /// The index config
    const albert::MatchConfig &config();

    /// Set the items to be indexed.
    /// @param items The items to be indexed.
    void setItems(std::vector<albert::IndexItem> &&items);

    /// Search the index for a string.
    /// @param string The string to search for.
    /// @param isValid A flag used to cancel the search.
    /// @return A list of scored items.
    std::vector<albert::RankItem> search(const QString &string, const bool &isValid) const;

private:

    class Private;
    std::unique_ptr<Private> d;

};
