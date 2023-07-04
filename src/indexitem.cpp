// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/indexitem.h"
using namespace std;

albert::IndexItem::IndexItem(shared_ptr<Item> i, QString s):
    item(::move(i)), string(::move(s))
{}
