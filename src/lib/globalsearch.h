// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensionwatcher.h"
#include "albert/globalqueryhandler.h"
#include <QString>
#include <vector>

struct GlobalSearch :
        public albert::GlobalQueryHandler,
        public albert::ExtensionWatcher<albert::GlobalQueryHandler>
{
    explicit GlobalSearch(albert::ExtensionRegistry&);
    QString id() const override;
    QString synopsis() const override;
    QString default_trigger() const override;
    bool allow_trigger_remap() const override;
    std::vector<albert::RankItem> rankItems(const Query &query) const override;
};

