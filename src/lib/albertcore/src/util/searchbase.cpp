// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QRegularExpression>
#include <QStringList>
#include <set>
#include "searchbase.h"
using std::set;

Core::SearchBase::~SearchBase() {

}

std::set<QString> Core::SearchBase::splitString(const QString &str) const {

    // Split the query into words W
    QStringList wordlist = str.toLower().split(QRegularExpression(SEPARATOR_REGEX), QString::SkipEmptyParts);

    // Make words unique
    set<QString> words(wordlist.begin(), wordlist.end());

    // Drop all words that are prefixes of others (since this an ordered set the next word)
    for (auto it = words.begin(); it != words.end();) {
        auto next = std::next(it);
        if ( next != words.end() && next->startsWith(*it) )
            it = words.erase(it);
        else
            ++it;
    }

    return words;
}
