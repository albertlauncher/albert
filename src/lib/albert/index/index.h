// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include <QString>
#include "vector"

namespace albert
{

class RankItem;
class IndexItem;

struct Index
{
    virtual ~Index() = default;
    virtual std::vector<RankItem> search(const QString &string) const = 0;
    virtual void setItems(std::vector<IndexItem>) = 0;
};

}
