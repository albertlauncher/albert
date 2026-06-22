// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "queryexecution.h"
#include <memory>
namespace albert
{
class QueryContext;
class GlobalQueryHandler;
}  // namespace albert

class GlobalQueryExecution final : public albert::QueryExecution
{
public:
    GlobalQueryExecution(albert::QueryContext context,
                         std::vector<albert::GlobalQueryHandler *> query_handlers);
    ~GlobalQueryExecution();

private:

    bool canFetchMore() const override;
    void fetchMore() override;
    void cancel() override;

    class Private;
    std::unique_ptr<Private> d;
};
