// Copyright (c) 2022 Manuel Schneider

#include "indexqueryhandlerprivate.h"
#include "indexqueryhandler.h"
#include "itemindex.h"
using namespace std;


albert::IndexQueryHandler::IndexQueryHandler() : d(new albert::IndexQueryHandler::Private(this)) {}

albert::IndexQueryHandler::~IndexQueryHandler() {}

void albert::IndexQueryHandler::updateIndex()
{
    d->updateIndex();
}

QString albert::IndexQueryHandler::synopsis() const
{
    return QStringLiteral("<filter>");
}

vector<albert::Match> albert::IndexQueryHandler::rankedItems(const albert::Query &query) const
{
    return d->index()->search(query.string());
}
