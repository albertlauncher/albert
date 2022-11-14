// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/indexqueryhandler.h"
#include "index.h"
using namespace std;
using namespace albert;


IndexItem::IndexItem(std::shared_ptr<Item> item, QString string):
    item(::move(item)), string(::move(string))
{

}


IndexQueryHandler::IndexQueryHandler() = default;

IndexQueryHandler::~IndexQueryHandler() = default;

QString IndexQueryHandler::synopsis() const
{
    return QStringLiteral("<filter>");
}

std::vector<RankItem> IndexQueryHandler::rankItems(const Query &query) const
{
    return index_->search(query.string());
}

void IndexQueryHandler::setIndex(unique_ptr<Index> &&index)
{
    index_ = ::move(index);
    updateIndex();
}

void IndexQueryHandler::updateIndex()
{
    if (index_)
        index_->setItems(indexItems());
}
