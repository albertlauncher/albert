// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "querycontext.h"
#include "queryexecution.h"
#include <memory>
namespace albert{ class GlobalQueryHandler; }

class GlobalQueryExecution final : public albert::QueryExecution, public albert::QueryContext
{
public:
    GlobalQueryExecution(albert::QueryContext &context,
                         std::vector<albert::GlobalQueryHandler *> query_handlers);
    ~GlobalQueryExecution();

private:
    // albert::Query
    // Required because we want to handle '*' as empty query and for atomic valid flag
    bool isValid() const override;
    const albert::QueryHandler &handler() const override;
    QString query() const override;
    QString trigger() const override;
    const albert::UsageScoring &usageScoring() const override;

    // albert::QueryExecution
    void cancel() override;
    void fetchMore() override;
    bool canFetchMore() const override;
    bool isActive() const override;

    class Private;
    std::unique_ptr<Private> d;
};
