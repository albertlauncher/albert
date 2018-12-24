// Copyright (C) 2014-2018 Manuel Schneider

#include <QDebug>
#include "albert/item.h"
#include "albert/query.h"
#include "matchcompare.h"


/** ***************************************************************************/
const QString &Core::Query::string() const {
    return string_;
}


/** ***************************************************************************/
const QString &Core::Query::rawString() const {
    return rawString_;
}


/** ***************************************************************************/
bool Core::Query::isTriggered() const {
    return !trigger_.isNull();
}


/** ***************************************************************************/
const QString &Core::Query::trigger() const {
    return trigger_;
}


/** ***************************************************************************/
bool Core::Query::isValid() const {
    return isValid_;
}


/** ***************************************************************************/
void Core::Query::addMatchWithoutLock(const std::shared_ptr<Core::Item> &item, uint score) {
    auto it = scores_.find(item->id());
    if ( it == scores_.end() )
        results_.emplace_back(item, score/2);
    else
        results_.emplace_back(item, (static_cast<ulong>(score)+it->second)/2);
}


/** ***************************************************************************/
void Core::Query::addMatchWithoutLock(std::shared_ptr<Core::Item> &&item, uint score) {
    auto it = scores_.find(item->id());
    if ( it == scores_.end() )
        results_.emplace_back(std::move(item), score/2);
    else
        results_.emplace_back(std::move(item), (static_cast<ulong>(score)+it->second)/2);
}
