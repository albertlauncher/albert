// Copyright (c) 2023-2024 Manuel Schneider

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
    if (score > other.score)
        return false;
    if (item->text().size() < other.item->text().size())
        return true;
    if (item->text().size() > other.item->text().size())
        return false;
    return item->text() > other.item->text();
}

bool albert::RankItem::operator>(const RankItem &other) const
{
    if (score > other.score)
        return true;
    if (score < other.score)
        return false;
    if (item->text().size() < other.item->text().size())
        return true;
    if (item->text().size() > other.item->text().size())
        return false;
    return item->text() < other.item->text();
}
