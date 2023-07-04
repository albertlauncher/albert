// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "triggerqueryhandler.h"
#include "globalqueryhandler.h"

namespace albert
{

/// Convenience GlobalQueryHandler providing generic TriggerQueryHandler
class ALBERT_EXPORT QueryHandler : public GlobalQueryHandler, public TriggerQueryHandler
{
public:
    /// Provides triggered query handling
    /// @implements QueryHandler::handleQuery
    /// @note Override this if handlers should behave differently when triggered
    void handleTriggerQuery(TriggerQuery *) const override;

};

}
