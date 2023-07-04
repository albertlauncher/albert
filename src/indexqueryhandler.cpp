// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "indexqueryhandlerprivate.h"
using namespace std;
using namespace albert;


IndexQueryHandler::IndexQueryHandler() : d(new IndexQueryHandlerPrivate(this)) {}

IndexQueryHandler::~IndexQueryHandler() = default;

void IndexQueryHandler::setIndexItems(vector<IndexItem> &&index_items)
{ d->setIndexItems(::move(index_items)); }

vector<RankItem> IndexQueryHandler::handleGlobalQuery(const GlobalQuery *query) const
{ return d->handleGlobalQuery(query); }
