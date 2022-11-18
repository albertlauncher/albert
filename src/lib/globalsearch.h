// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/globalqueryhandler.h"
#include "albert/util/extensionwatcher.h"
#include <QString>
#include <set>

struct GlobalSearch:
        public albert::GlobalQueryHandler,
        public albert::ExtensionWatcher<albert::GlobalQueryHandler>

{
    explicit GlobalSearch(albert::ExtensionRegistry&);

    // GlobalQueryHandler
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString default_trigger() const override;
    bool allow_trigger_remap() const override;
    std::vector<albert::RankItem> rankItems(const Query &query) const override;

    // ExtensionWatcher
    void onAdd(albert::GlobalQueryHandler *h) override {handlers_.insert(h);}
    void onRem(albert::GlobalQueryHandler *h) override {handlers_.erase(h);}

private:
    std::set<albert::GlobalQueryHandler*> handlers_;
};

