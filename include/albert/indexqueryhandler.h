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
/// A \ref GlobalQueryHandler providing implicit indexing and matching.
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

    /// Returns the matching items for _context_.
    std::vector<RankItem> rankItems(QueryContext &context) override;

    ///
    /// Updates the index.
    ///
    /// Called when the index needs to be updated, i.e. for initialization, on user changes to the
    /// index config (fuzzy, etc…) and probably by the client itself if the items changed. This
    /// function should call \ref setIndexItems(std::vector<IndexItem>&&) to update the index.
    ///
    /// @note Do not call this method in the constructor. It will be called on plugin
    /// initialization.
    ///
    virtual void updateIndexItems() = 0;

    /// Sets the items of the index to _items_.
    void setIndexItems(std::vector<IndexItem> &&items);

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
