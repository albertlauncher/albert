// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
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
    std::vector<albert::RankItem> handleGlobalQuery(const albert::GlobalQueryHandler::GlobalQuery*) const;
    void setIndexItems(std::vector<albert::IndexItem> &&);

private:
    albert::IndexQueryHandler * const q;
    std::unique_ptr<Index> index_;
};

