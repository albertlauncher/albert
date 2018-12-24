// Copyright (C) 2014-2018 Manuel Schneider

#include "albert/item.h"
#include "matchcompare.h"
using namespace std;

/** ***************************************************************************/
bool Core::MatchCompare::operator()(const pair<shared_ptr<Item>, uint> &lhs,
                                  const pair<shared_ptr<Item>, uint> &rhs) {
    // Compare urgency then score
    if (lhs.first->urgency() != rhs.first->urgency())
        return lhs.first->urgency() < rhs.first->urgency();
    else if (lhs.second != rhs.second)
        return lhs.second > rhs.second; // Compare match score
    else
        return lhs.first->text().size() < rhs.first->text().size();
}
