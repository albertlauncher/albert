// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/query/indexitem.h"
using namespace std;

albert::IndexItem::IndexItem(shared_ptr<Item> i, QString s):
    item(::move(i)), string(::move(s))
{}
