// Copyright (c) 2023-2025 Manuel Schneider

#include "rankitem.h"
using namespace std;

albert::RankItem::RankItem(shared_ptr<Item> &&i, double s) noexcept:
    item(::move(i)), score(s) {}

albert::RankItem::RankItem(const shared_ptr<Item> &i, double s) noexcept:
    item(i), score(s) {}

bool albert::RankItem::operator<(const RankItem &other) const
{
    if (score < other.score)
        return true;
    else if (score > other.score)
        return false;
    else if (const auto lt = item->text(), rt = other.item->text();
             lt.size() > rt.size())
        return true;
    else if (lt.size() < rt.size())
        return false;
    else
        return lt > rt;
}

bool albert::RankItem::operator>(const RankItem &other) const
{
    if (score > other.score)
        return true;
    else if (score < other.score)
        return false;
    else if (const auto lt = item->text(), rt = other.item->text();
             lt.size() < rt.size())
        return true;
    else if (lt.size() > rt.size())
        return false;
    else
        return lt < rt;
}
