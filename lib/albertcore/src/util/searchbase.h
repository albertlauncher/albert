// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QString>
#include <vector>
#include <set>
#include <memory>

namespace Core {

class IndexableItem;

class SearchBase
{
public:

    virtual ~SearchBase();
    virtual void add(const std::shared_ptr<IndexableItem> &idxble) = 0;
    virtual void clear() = 0;
    virtual std::vector<std::shared_ptr<IndexableItem>> search(const QString &req) const = 0;

protected:

    std::set<QString> splitString(const QString &) const;

    static constexpr const char* SEPARATOR_REGEX  = "[!?<>\"'=+*.:,;\\\\\\/ _\\-]+";

};

}
