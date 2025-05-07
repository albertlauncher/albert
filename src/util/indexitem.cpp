// Copyright (c) 2021-2024 Manuel Schneider

#include "indexitem.h"
using namespace albert;
using namespace std;
using namespace util;

IndexItem::IndexItem(shared_ptr<Item> i, QString s):
    item(::move(i)), string(::move(s)){}
