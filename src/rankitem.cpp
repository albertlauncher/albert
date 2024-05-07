// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/query/rankitem.h"
using namespace std;

albert::RankItem::RankItem(shared_ptr<Item> i, double s):
    item(::move(i)), score(s)
{}
