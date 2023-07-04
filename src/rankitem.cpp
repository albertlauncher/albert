// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/rankitem.h"
using namespace std;


albert::RankItem::RankItem(shared_ptr<Item> i, float s):
    item(::move(i)), score(s)
{}
