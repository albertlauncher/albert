// Copyright (c) 2023-2025 Manuel Schneider

#include "indexqueryhandler.h"
#include "itemindex.h"
#include "query.h"
#include <mutex>
#include <shared_mutex>
using namespace albert;
using namespace std;
using namespace util;

class IndexQueryHandler::Private
{
public:
    shared_mutex index_mutex;
    ItemIndex index;
};

IndexQueryHandler::IndexQueryHandler() : d(new Private()) {}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(vector<IndexItem> &&index_items)
{
    scoped_lock l(d->index_mutex);
    d->index.setItems(::move(index_items));
}

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const Query &query)
{
    shared_lock l(d->index_mutex);
    return d->index.search(query.string(), query.isValid());
}

bool IndexQueryHandler::supportsFuzzyMatching() const { return true; }

void IndexQueryHandler::setFuzzyMatching(bool fuzzy)
{
    d->index_mutex.lock();
    if (d->index.config().fuzzy != fuzzy)
    {
        d->index = ItemIndex({.fuzzy = fuzzy});
        d->index_mutex.unlock();
        updateIndexItems();
    }
    else
        d->index_mutex.unlock();
}
