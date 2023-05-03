// Copyright (c) 2023 Manuel Schneider

#include "indexqueryhandlerprivate.h"
using namespace std;
using namespace albert;

IndexQueryHandlerPrivate::IndexQueryHandlerPrivate(IndexQueryHandler *q_) : q(q_) {}

IndexQueryHandlerPrivate::~IndexQueryHandlerPrivate() = default;

void IndexQueryHandlerPrivate::setIndex(unique_ptr<Index> &&index)
{
    index_ = ::move(index);
    q->updateIndexItems();
}

void IndexQueryHandlerPrivate::setIndexItems(std::vector<IndexItem> &&index_items)
{
    index_->setItems(::move(index_items));
}

std::vector<RankItem>
IndexQueryHandlerPrivate::handleGlobalQuery(const GlobalQueryHandler::GlobalQuery &query) const
{
    return index_->search(query.string(), query.isValid());
}
