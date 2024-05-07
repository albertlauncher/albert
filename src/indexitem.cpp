// Copyright (c) 2021-2024 Manuel Schneider

#include "albert/util/indexitem.h"
using namespace std;

albert::IndexItem::IndexItem(shared_ptr<Item> i, QString s):
    item(::move(i)), string(::move(s)){}
