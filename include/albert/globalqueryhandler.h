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
/// Query handler participating in the global search.
///
/// By design, every global query handler is also a triggered query handler. Therefore this class
/// inherits \ref RankedQueryHandler and as such inherits its contract. I.e. the handler returns a
/// complete set of match-scored items eagerly. The provided match scores will be combined with the
/// usage-based scoring weighted by user configuration. Finally the items (of all global handlers)
/// will be yielded lazily in order of their final score.
///
/// Note: Global queries are expected to complete within a few milliseconds.
///
/// \ingroup core_extension
///
class ALBERT_EXPORT GlobalQueryHandler : public albert::RankedQueryHandler
{
public:
    ///
    /// Returns a list of special items that should show up on an emtpy query.
    ///
    /// The empty pattern matches everything. For triggered queries this is desired and by design
    /// lots of triggered handlers reuse GlobalQueryHandler::rankItems. The empty global query is
    /// not executed. This function allows dedicated empty global query handling.
    ///
    virtual std::vector<std::shared_ptr<Item>> handleEmptyQuery();

protected:
    /// Destructs the handler.
    ~GlobalQueryHandler() override;
};

}  // namespace albert
