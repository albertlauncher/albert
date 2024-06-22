// Copyright (c) 2023-2024 Manuel Schneider

#include "indexqueryhandler.h"
#include "itemindex.h"
#include "query.h"
#include <memory>
#include <mutex>
#include <shared_mutex>
using namespace albert;
using namespace std;

class IndexQueryHandler::Private
{
public:
    ItemIndex index;
    std::shared_mutex index_mutex;
};

IndexQueryHandler::IndexQueryHandler() : d(new Private) {}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(vector<IndexItem> &&index_items)
{
    unique_lock l(d->index_mutex);
    d->index.setItems(::move(index_items));
}

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const Query *query) const
{
    shared_lock l(d->index_mutex);
    return d->index.search(query->string(), query->isValid());
}

bool IndexQueryHandler::supportsFuzzyMatching() const { return true; }

void IndexQueryHandler::setFuzzyMatching(bool fuzzy)
{
    if ((bool)d->index.config().error_tolerance_divisor == fuzzy)
    {
        auto c = d->index.config();

        if (!fuzzy)
            c.error_tolerance_divisor = 0;

        d->index_mutex.lock();
        d->index = ItemIndex(c);
        d->index_mutex.unlock();

        updateIndexItems();
    }
}
