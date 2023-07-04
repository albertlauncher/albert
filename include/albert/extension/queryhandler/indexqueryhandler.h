// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "indexitem.h"
#include "queryhandler.h"
#include <memory>
#include <vector>
class IndexQueryHandlerPrivate;

namespace albert
{

/// Global search index query handler. This is a GlobalQueryHandler which does indexing and
/// matching for you. You just have to provide your items with lookup strings.
class ALBERT_EXPORT IndexQueryHandler : public QueryHandler
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

    /// Set the items of the index. You should probably call this in updateIndexItems().
    void setIndexItems(std::vector<IndexItem>&&);

    /// @implements GlobalQueryHandler::handleQuery
    /// Uses the index to find items
    std::vector<RankItem> handleGlobalQuery(const GlobalQuery*) const override;

    IndexQueryHandlerPrivate * const d; ///< Do not touch
};

}
