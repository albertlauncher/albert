// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/rankitem.h>
#include <albert/threadedqueryhandler.h>
#include <memory>
#include <vector>

namespace albert
{

///
/// Abstract global query handler.
///
/// A functional query handler returning scored items. Applicable for the
/// global search. Use this if you want your results show up in the global
/// search.
///
/// By design choice every global query handler should also provide an exclusive handler. To enforce
/// this GlobalQueryHandler inherits \ref ThreadedQueryHandler and implements the \ref
/// ThreadedQueryHandler::handleThreadedQuery.
///
/// @note Do _not_ use this for long running tasks!
///
/// \ingroup core
///
class ALBERT_EXPORT GlobalQueryHandler : public albert::ThreadedQueryHandler
{
public:

    ///
    /// Returns a list of items matching the _query_ or all items if the query is empty.
    ///
    /// The match score should make sense and often (if not always) be the fraction of matched chars
    /// (legth of query string / length of item title). The empty query should return all items with
    /// a score of 0.
    ///
    /// @note Executed in a worker thread.
    ///
    virtual std::vector<RankItem> handleGlobalQuery(Query &query) = 0;

    ///
    /// Returns a list of special items that should show up on an emtpy query.
    ///
    /// Empty patterns match everything. For triggered queries this is desired and by design lots of
    /// handlers relay the handleThreadedQuery to handleGlobalQuery. For global queries this leads to
    /// an expensive query execution on empty queries. Therefore the empty global query is not
    /// executed. This function allows dedicated empty global query handling.
    ///
    virtual std::vector<std::shared_ptr<Item>> handleEmptyQuery();

protected:
    /// Destructs the handler.
    ~GlobalQueryHandler() override;

    ///
    /// Calls \ref handleGlobalQuery, \ref applyUsageScore, sorts and adds the items to _query_.
    ///
    /// @note Reimplement if the handler should have custom triggered behavior.
    /// Think twice though, it may break user expectation.
    ///
    void handleThreadedQuery(ThreadedQuery &query) override;

};

}
