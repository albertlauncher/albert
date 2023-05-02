// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extensions/queryhandler.h"
#include "globalqueryhandlerprivate.h"
#include "indexqueryhandlerprivate.h"
#include <utility>
using namespace std;
using namespace albert;

///////////////////////////////////////////////////////////////////////////////////////////////////

QueryHandler::Query::~Query() = default;

QString QueryHandler::synopsis() const { return {}; }

QString QueryHandler::defaultTrigger() const { return QString("%1 ").arg(id()); }

bool QueryHandler::allowTriggerRemap() const { return true; }

///////////////////////////////////////////////////////////////////////////////////////////////////

RankItem::RankItem(shared_ptr<Item> i, Score s):
    item(std::move(i)), score(s) {}

GlobalQueryHandler::Query::~Query() = default;

GlobalQueryHandler::GlobalQueryHandler() : d(new GlobalQueryHandlerPrivate(this)) {}

GlobalQueryHandler::~GlobalQueryHandler() = default;

void GlobalQueryHandler::handleQuery(QueryHandler::Query &query) const
{
    std::vector<RankItem> &&rank_items = d->handleQuery(dynamic_cast<Query&>(query));
    sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){ return a.score > b.score; });

    // TODO c++20 ranges::view
    std::vector<shared_ptr<Item>> items;
    items.reserve(rank_items.size());
    for (auto &match : rank_items)
        items.push_back(std::move(match.item));

    query.add(std::move(items));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IndexItem::IndexItem(std::shared_ptr<Item> i, QString s):
    item(::move(i)), string(::move(s)){}

IndexQueryHandler::IndexQueryHandler() : d(new IndexQueryHandlerPrivate(this)) {}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(std::vector<IndexItem> &&index_items)
{ d->setIndexItems(::move(index_items)); }

std::vector<RankItem> IndexQueryHandler::handleQuery(const GlobalQueryHandler::Query &query) const
{ return d->handleQuery(query); }
