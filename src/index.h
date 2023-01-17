// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include <QString>
#include <vector>
namespace albert {
    class RankItem;
    class IndexItem;
}

class Index
{
public:
    virtual ~Index() = default;
    virtual std::vector<albert::RankItem> search(const QString &string, const bool &isValid) const = 0;
    virtual void setItems(std::vector<albert::IndexItem> &&) = 0;
};
