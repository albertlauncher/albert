// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "globalqueryhandler.h"
#include <QCoreApplication>
class QueryEngine;

class TriggersQueryHandler : public albert::GlobalQueryHandler
{
    Q_DECLARE_TR_FUNCTIONS(TriggersQueryHandler)

public:

    TriggersQueryHandler(const QueryEngine &query_engine);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void handleTriggerQuery(albert::Query *) override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query*) const override;

private:

    static const QStringList icon_urls;
    const QueryEngine &query_engine_;

};
