// SPDX-FileCopyrightText: 2026 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/generatorqueryhandler.h>
#include <albert/rankitem.h>
#include <memory>
#include <vector>

namespace albert
{

///
/// Query handler participating in the global search.
///
/// Implementations have to implement \ref rankItems which will be called for each global query.
/// The match score of each returned items will be combined with its usage score weighted according
/// to the user configuration. Finally the items of all global handlers will be sorted lazily in
/// order of their final score.
///
/// Warning: Global queries must return fast to guarantee a lag free user experience.
///
/// By design, every global query handler must also provide triggered query handling. Therefore this
/// class inherits \ref GeneratorQueryHandler and implements \ref items by returning the results of
/// \ref rankItems with usage scoring applied and lazily sorted.
///
/// Note: If you want to provide custom triggered query handling, reimplement \ref items.
///
/// \ingroup core_extension
///
class ALBERT_EXPORT GlobalQueryHandler : public albert::GeneratorQueryHandler
{
public:
    ///
    /// Returns a list of scored matches for _context_.
    ///
    /// The match score should make sense and often is the fraction of matched characters (legth of
    /// query string / length of matched string). The empty pattern matches everything and returns
    /// all items with a score of 0.
    ///
    /// \note Executed in a background thread.
    ///
    virtual std::vector<RankItem> rankItems(QueryContext &context) = 0;

    ///
    /// Returns a list of special items that should show up on an emtpy query.
    ///
    /// The empty pattern matches everything. For triggered queries this is desired and by design
    /// lots of triggered handlers reuse GlobalQueryHandler::rankItems. The empty global query is
    /// not executed. This function allows dedicated empty global query handling.
    ///
    /// The base implementation returns an empty list.
    ///
    virtual std::vector<std::shared_ptr<Item>> handleEmptyQuery();

    /// Yields result of \ref rankItems for _context_ with usage scoring applied and lazily sorted.
    ItemGenerator items(QueryContext &context) override;

protected:
    /// Destructs the handler.
    ~GlobalQueryHandler() override;
};

}  // namespace albert
