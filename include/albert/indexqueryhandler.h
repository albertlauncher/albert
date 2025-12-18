// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/globalqueryhandler.h>
#include <albert/indexitem.h>
#include <memory>
#include <vector>

namespace albert
{

///
/// Index-based global query handler.
///
/// Convenience base class for global query handlers backed by a precomputed index. Query execution
/// is performed against the index and provides fast, deterministic matching suitable for the global
/// search. Implementations are responsible for providing the indexed items and keeping the index up
/// to date when the underlying data changes.
///
/// \ingroup util_query
///
class ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
public:
    /// Returns `true`
    bool supportsFuzzyMatching() const override;

    /// Sets the fuzzy matching mode to _enabled_ and triggers \ref updateIndexItems().
    void setFuzzyMatching(bool enabled) override;

    /// Returns a list of scored matches for _context_ using the index.
    std::vector<RankItem> rankItems(QueryContext &context) override;

    ///
    /// Updates the index.
    ///
    /// Called when the index needs to be updated, i.e. for initialization, on user changes to the
    /// index config (fuzzy, etc…) and probably by the client itself if the items changed. This
    /// function should call \ref setIndexItems to update the index.
    ///
    /// @note Do not call this method on plugin initialization. It will be called once loaded.
    ///
    virtual void updateIndexItems() = 0;

    /// Sets the items of the index to _index_items_.
    void setIndexItems(std::vector<IndexItem> &&index_items);

protected:
    /// Constructs an index query handler.
    IndexQueryHandler();

    /// Destructs the index query handler.
    ~IndexQueryHandler() override;

private:
    class Private;
    std::unique_ptr<Private> d;

};

}
