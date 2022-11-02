// Copyright (c) 2022 Manuel Schneider

#include "../scopedtimeprinter.hpp"
#include "albert/item.h"
#include "albert/query.h"
#include "albert/queryhandler.h"
#include <vector>
using namespace std;
using namespace albert;

const QString &Query::trigger() const { return trigger_; }

const QString &Query::string() const { return string_; }

bool Query::isValid() const { return valid_; }

bool Query::isFinished() const { return finished_; }

const vector<shared_ptr<Item>> &Query::results() const { return results_; }

void Query::add_(const shared_ptr<Item> &item) { results_.push_back(item); }

void Query::add_(shared_ptr<Item> &&item) { results_.push_back(::move(item)); }

void Query::set(vector<shared_ptr<Item>> &&items) { results_ = ::move(items); emit resultsChanged(); }

void Query::activateResult(uint item, uint action)
{
    results_[item]->actions()[action].function();
    // todo database
}
