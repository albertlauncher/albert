// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/query/globalqueryhandler.h"
#include "itemindex.h"
#include <QString>
#include <memory>
#include <vector>

namespace albert
{

/// Index query handler class.
/// A GlobalQueryHandler providing implicit indexing and matching.
/// You just have to provide your items with lookup strings.
class ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
public:
    IndexQueryHandler();
    ~IndexQueryHandler() override;

    /// Returns "True"
    bool supportsFuzzyMatching() const override;

    /// Set the fuzzy mode of the internal index.
    /// Triggers a rebuild by calling updateIndexItems.
    void setFuzzyMatching(bool) override;

    /// Uses the index to override GlobalQueryHandler::handleGlobalQuery
    std::vector<RankItem> handleGlobalQuery(const Query*) const override;

    /// Update the index. Called when the index needs to be updated (or probably by yourself if
    /// your items changed), i.e. whenever the user made changes to the index config or initially
    /// on creation. Don't call in the constructor. It will be called on plugin initialization.
    /// @see void IndexQueryHandler::setIndexItems(std::vector<IndexItem>&&)
    virtual void updateIndexItems() = 0;

    /// Set the items of the index. Call this in updateIndexItems(). @threadsafe
    void setIndexItems(std::vector<IndexItem>&&);

private:
    class Private;
    std::unique_ptr<Private> d;
};

}
