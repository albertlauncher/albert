// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "indexitem.h"
#include "globalqueryhandler.h"
#include <memory>
#include <vector>
class IndexQueryHandlerPrivate;

namespace albert
{

/// Index query handler class.
/// A QueryHandler that providing implicit indexing and matching. You just have
/// to provide your items with lookup strings.
class ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
public:
    IndexQueryHandler();
    ~IndexQueryHandler() override;

    /// Update the index. Called when the index needs to be updated (or probably by yourself if
    /// your items changed), i.e. whenever the user made changes to the index config or initially
    /// on creation. Therefore…
    /// @note You dont have to call this in you constructor. It will be called after construction.
    /// @see void IndexQueryHandler::setIndexItems(std::vector<IndexItem>&&)
    virtual void updateIndexItems() = 0;

    /// Set the items of the index. Call this in updateIndexItems().
    void setIndexItems(std::vector<IndexItem>&&);

    /// "<filter>" default synopsis
    QString synopsis() const override;

    /// Returns "True"
    bool supportsFuzzyMatching() const override final;

    /// Return the fuzzy mode of the internal index
    bool fuzzyMatchingEnabled() const override final;

    /// Set the fuzzy mode of the internal index. Triggers a rebuild
    void setFuzzyMatchingEnabled(bool) override final;

    /// Use the index to override handleGlobalQuery
    std::vector<RankItem> handleGlobalQuery(const GlobalQuery*) const override;

private:
    std::unique_ptr<IndexQueryHandlerPrivate> d;
};

}
