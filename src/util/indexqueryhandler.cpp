// Copyright (c) 2023-2024 Manuel Schneider

#include "indexqueryhandler.h"
#include "itemindex.h"
#include "query.h"
#include <memory>
#include <mutex>
#include <shared_mutex>
using namespace albert;
using namespace std;
using namespace util;

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
    // Pointer check not necessary since never called before setFuzzyMatching
    unique_lock l(d->index_mutex);
    d->index->setItems(::move(index_items));
}

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const Query &query)
{
    // Pointer check not necessary since never called before setFuzzyMatching
    shared_lock l(d->index_mutex);
    return d->index->search(query.string(), query.isValid());
}

bool IndexQueryHandler::supportsFuzzyMatching() const { return true; }

void IndexQueryHandler::setFuzzyMatching(bool fuzzy)
{
    if (!d->index)
    {
        auto c = MatchConfig{.fuzzy = fuzzy};
        d->index = make_unique<ItemIndex>(c);
        updateIndexItems();
    }
    else if ((bool)d->index->config().fuzzy != fuzzy)
    {
        auto c = d->index->config();
        c.fuzzy = fuzzy;

        d->index_mutex.lock();
        d->index = make_unique<ItemIndex>(c);
        d->index_mutex.unlock();
        updateIndexItems();
    }
}
