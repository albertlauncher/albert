// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "queryhandler.h"
#include <QString>
#include <map>
namespace albert { class GlobalQueryHandler; }

class GlobalQuery : public albert::QueryHandler
{
public:

    std::map<QString, albert::GlobalQueryHandler*> global_query_handlers;

private:

    QString id() const override;
    QString name() const override;
    QString description() const override;
    std::unique_ptr<albert::QueryExecution> execution(albert::Query &query) override;

};
