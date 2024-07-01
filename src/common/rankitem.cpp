// Copyright (c) 2023-2024 Manuel Schneider

#include "matcher.h"
#include "rankitem.h"
using namespace std;

albert::RankItem::RankItem(shared_ptr<Item> &&i, double s):
    item(::move(i)), score(s) {}

albert::RankItem::RankItem(const shared_ptr<Item> &i, double s):
    item(i), score(s) {}

albert::RankItem::RankItem(shared_ptr<Item> &&i, Match m):
    item(::move(i)), score(m.score()) {}

albert::RankItem::RankItem(const shared_ptr<Item> &i, Match m):
    item(i), score(m.score()) {}

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
