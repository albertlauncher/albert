// Copyright (c) 2023-2024 Manuel Schneider

#include "query.h"
#include "indexqueryhandler.h"
#include "itemindex.h"
#include <memory>
#include <mutex>
#include <shared_mutex>
using namespace albert;
using namespace std;

static const uint GRAM_SIZE = 2;
//static const char* CFG_SEPARATORS = "separators";
static const char* DEF_SEPARATORS = R"R([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)R";
static const uint DEF_ERROR_TOLERANCE_DIVISOR = 4;

class IndexQueryHandler::Private
{
public:
    std::unique_ptr<ItemIndex> index;
    std::shared_mutex index_mutex;
};

IndexQueryHandler::IndexQueryHandler() : d(new Private) {}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(vector<IndexItem> &&index_items)
{
    unique_lock l(d->index_mutex);
    d->index->setItems(::move(index_items));
}

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const Query *query) const
{
    shared_lock l(d->index_mutex);
    return d->index->search(query->string(), query->isValid());
}

bool IndexQueryHandler::supportsFuzzyMatching() const { return true; }

void IndexQueryHandler::setFuzzyMatching(bool value)
{
    d->index_mutex.lock();
    d->index = make_unique<ItemIndex>(
        DEF_SEPARATORS, false, GRAM_SIZE,
        value ? DEF_ERROR_TOLERANCE_DIVISOR : 0
    );
    d->index_mutex.unlock();
    updateIndexItems();
}
