// Copyright (c) 2026 Manuel Schneider

#pragma once
#include <QString>
#include <QHash>

class ItemKey
{
public:
    QString extension_id;
    QString item_id;
    bool operator==(const ItemKey&) const = default;
};

// Hashing specialization for ItemKey
template <>
struct std::hash<ItemKey>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline std::size_t operator()(const ItemKey& key) const
    { return (qHash(key.extension_id) ^ (qHash(key.item_id)<< 1)); }
};
