// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/rankedqueryhandler.h>
#include <albert/rankitem.h>
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
/// \ingroup core_extension
///
class ALBERT_EXPORT GlobalQueryHandler : public albert::RankedQueryHandler
{
public:
    ///
    /// Returns a list of special items that should show up on an emtpy query.
    ///
    /// Empty patterns match everything. For triggered queries this is desired and by design lots of
    /// handlers relay the handleThreadedQuery to handleGlobalQuery. For global queries this leads
    /// to an expensive query execution on empty queries. Therefore the empty global query is not
    /// executed. This function allows dedicated empty global query handling.
    ///
    virtual std::vector<std::shared_ptr<Item>> handleEmptyQuery();

protected:
    /// Destructs the handler.
    ~GlobalQueryHandler() override;
};

}  // namespace albert
