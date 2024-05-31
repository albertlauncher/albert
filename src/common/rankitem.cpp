// Copyright (c) 2023-2024 Manuel Schneider

#include "matcher.h"
#include "rankitem.h"
using namespace std;

albert::RankItem::RankItem(shared_ptr<Item> &&i, double s):
    item(::move(i)), score(s) {}

albert::RankItem::RankItem(const shared_ptr<Item> &i, double s):
    item(i), score(s) {}

albert::RankItem::RankItem(shared_ptr<Item> &&i, Match &m):
    item(::move(i)), score(m.score()) {}

albert::RankItem::RankItem(const shared_ptr<Item> &i, Match &m):
    item(i), score(m.score()) {}
