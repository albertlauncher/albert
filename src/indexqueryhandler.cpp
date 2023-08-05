// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "indexqueryhandlerprivate.h"
#include "itemindex.h"
#include <mutex>
using namespace std;
using namespace albert;
static const uint GRAM_SIZE = 2;
//static const char* CFG_SEPARATORS = "separators";
static const char* DEF_SEPARATORS = R"R([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)R";
static const uint DEF_ERROR_TOLERANCE_DIVISOR = 4;

IndexQueryHandler::IndexQueryHandler() : d(new IndexQueryHandlerPrivate)
{
    class NullIndex : public Index
    {
    public:
        vector<RankItem> search(const QString&, const bool&) const override { return {}; }
        void setItems(vector<IndexItem> &&) override {}
    };
    d->index = make_unique<NullIndex>();
}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(vector<IndexItem> &&index_items)
{
    unique_lock l(d->index_mutex);
    d->index->setItems(::move(index_items));
}

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const GlobalQuery *query) const
{
    shared_lock l(d->index_mutex);
    return d->index->search(query->string(), query->isValid());
}

QString IndexQueryHandler::synopsis() const { return QStringLiteral("<filter>"); }

bool IndexQueryHandler::supportsFuzzyMatching() const { return true; }

bool IndexQueryHandler::fuzzyMatching() const { return d->fuzzy; }

void IndexQueryHandler::setFuzzyMatching(bool value)
{
    d->fuzzy = value;
    d->index_mutex.lock();
    d->index = make_unique<ItemIndex>(
        DEF_SEPARATORS, false, GRAM_SIZE,
        value ? DEF_ERROR_TOLERANCE_DIVISOR : 0
    );
    d->index_mutex.unlock();
    updateIndexItems();
}
