// Copyright (c) 2023-2024 Manuel Schneider

#include "indexqueryhandler.h"
#include "itemindex.h"
#include "query.h"
#include <memory>
#include <mutex>
#include <shared_mutex>
using namespace albert::util;
using namespace albert;
using namespace std;

class IndexQueryHandler::Private
{
public:
    unique_ptr<ItemIndex> index;
    std::shared_mutex index_mutex;
};

IndexQueryHandler::IndexQueryHandler() : d(new Private()) {}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(vector<IndexItem> &&index_items)
{
    scoped_lock l(d->index_mutex);
    if (d->index)
        d->index->setItems(::move(index_items));
}

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const Query &query)
{
    shared_lock l(d->index_mutex);
    if (d->index)
        return d->index->search(query.string(), query.isValid());
    return {};
}

bool IndexQueryHandler::supportsFuzzyMatching() const { return true; }

void IndexQueryHandler::setFuzzyMatching(bool fuzzy)
{
    d->index_mutex.lock();
    if (!d->index  // lazy index init
        || d->index->config().fuzzy != fuzzy)
    {
        d->index = make_unique<ItemIndex>(MatchConfig{.fuzzy = fuzzy});
        d->index_mutex.unlock();
        updateIndexItems();
    }
    else
        d->index_mutex.unlock();
}
