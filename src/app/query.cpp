// Copyright (C) 2014-2018 Manuel Schneider

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
void Core::Query::disableSort()
{
    sort_ = false;
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
        results_.emplace_back(item, 0 /*score/2*/);
    else
        results_.emplace_back(item, it->second/*(static_cast<ulong>(score)+it->second)/2*/);
}


/** ***************************************************************************/
void Core::Query::addMatchWithoutLock(std::shared_ptr<Core::Item> &&item, uint score) {
    auto it = scores_.find(item->id());
    if ( it == scores_.end() )
        results_.emplace_back(std::move(item), 0/*score/2*/);
    else
        results_.emplace_back(std::move(item), it->second/*(static_cast<ulong>(score)+it->second)/2*/);
}
