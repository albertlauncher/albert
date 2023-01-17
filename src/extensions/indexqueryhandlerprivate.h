// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extensions/queryhandler.h"
#include "index.h"
#include <QString>
#include <memory>
#include <vector>

class IndexQueryHandlerPrivate final
{
public:
    IndexQueryHandlerPrivate(albert::IndexQueryHandler *q);
    ~IndexQueryHandlerPrivate();

    void setIndex(std::unique_ptr<Index>&&);
    std::vector<albert::RankItem> handleQuery(const albert::GlobalQueryHandler::Query&) const;
    void setIndexItems(std::vector<albert::IndexItem> &&);

private:
    albert::IndexQueryHandler *q;
    std::unique_ptr<Index> index_;
};

