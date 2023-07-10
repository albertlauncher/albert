// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include "rankitem.h"
#include <vector>
class GlobalQueryHandlerPrivate;

namespace albert
{

/// Global search query handler.
/// Use this if you want your results appear in the global, untriggered search.
/// @note Inherits TriggeredQueryHandler, therefore this extension also handles triggers.
/// @note Do _not_ use this for long running tasks! @see TriggeredQueryHandler
class ALBERT_EXPORT GlobalQueryHandler : virtual public Extension
{
public:
    GlobalQueryHandler();
    ~GlobalQueryHandler() override;

    struct GlobalQuery
    {
        virtual ~GlobalQuery();
        virtual const QString string() const = 0;  ///< The query string
        virtual bool isValid() const = 0;  ///< True if query has not been cancelled
    };

    /// The query handling function. Subclasses should return matched items with appropriate match scores.
    /// The match score should make sense and often (if not always) the fraction of the string match makes sense.
    /// @note has to be thread safe!
    virtual std::vector<RankItem> handleGlobalQuery(const GlobalQuery*) const = 0;

    GlobalQueryHandlerPrivate * const d; ///< Do not touch
};

}
