// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "indexqueryhandlerprivate.h"
#include "itemindex.h"
using namespace std;
using namespace albert;
static const uint GRAM_SIZE = 2;
//static const char* CFG_SEPARATORS = "separators";
static const char* DEF_SEPARATORS = R"R([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)R";
static const uint DEF_ERROR_TOLERANCE_DIVISOR = 4;

IndexQueryHandler::IndexQueryHandler() : d(new IndexQueryHandlerPrivate)
{
    d->index = make_unique<ItemIndex>(
        DEF_SEPARATORS, false, GRAM_SIZE,
        fuzzyMatchingEnabled() ? DEF_ERROR_TOLERANCE_DIVISOR : 0
    );
}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(vector<IndexItem> &&index_items)
{ d->index->setItems(::move(index_items)); }

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const GlobalQuery *query) const
{return d->index->search(query->string(), query->isValid()); }

QString IndexQueryHandler::synopsis() const { return QStringLiteral("<filter>"); }

bool IndexQueryHandler::supportsFuzzyMatching() const { return true; }

bool IndexQueryHandler::fuzzyMatchingEnabled() const { return d->fuzzy; }

void IndexQueryHandler::setFuzzyMatchingEnabled(bool value)
{
    d->fuzzy = value;
    d->index = make_unique<ItemIndex>(
        DEF_SEPARATORS, false, GRAM_SIZE,
        value ? DEF_ERROR_TOLERANCE_DIVISOR : 0
    );
    updateIndexItems();
}
