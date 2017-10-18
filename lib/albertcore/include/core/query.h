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

#pragma once
#include <QMutex>
#include <QString>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include "core_globals.h"


namespace Core {

class QueryPrivate;
class Item;

/**
 * @brief The Query class
 * Represents a query to be handled by the query handlers
 */
class EXPORT_CORE Query final
{
public:

    /**
     * @brief The query string.
     * This is the processed query string relevant for most of the handlers. If the raw query string
     * is prefixed by a trigger string() returns the query string without the the trigger
     * prefix. If there is no trigger string() is equivalent to rawString().
     */
    const QString &string() const;

    /**
     * @brief The raw query string.
     * This is the raw query string as the users entered it into the input line.
     */
    const QString &rawString() const;

    /**
     * @brief Indicates a triggered query.
     */
    bool isTriggered() const;

    /**
     * @brief The trigger of this query
     * Note that if the trigger is set, string() differs from rawString().
     */
    const QString &trigger() const;

    /**
     * @brief The validity of the query
     * If the core cancelled the query for some reason the query gets invalid. Stop handling the
     * query if it is not valid anymore to save resources for other queries.
     * @return True if valid, else false.
     */
    bool isValid() const;

    /**
     * @brief addMatch
     * Use the addMatches if you have a lot of items to add.
     * @param item The to add to the results
     * @param score The relevance factor (UINT_MAX -> 1)
     * @see addMatches
     */
    template<typename T>
    void addMatch(T&& item, uint score = 0) {
        if ( isValid_ ) {
            mutex_.lock();
            addMatchWithoutLock(std::forward<T>(item), score);
            mutex_.unlock();
        }
    }

    /**
     * @brief addMatches
     * Cumulative addMatch function avoiding excessive mutex locking the results
     * @param begin
     * @param end
     */
    template<typename Iterator>
    void addMatches(Iterator begin, Iterator end) {
        if ( isValid() ) {
            mutex_.lock();
            for (; begin != end; ++begin)
                // Must not use operator->() !!! dereferencing a pointer returns an lvalue
                addMatchWithoutLock((*begin).first, (*begin).second);
            mutex_.unlock();
        }
    }

private:

    void addMatchWithoutLock(const std::shared_ptr<Core::Item> &item, uint score);
    void addMatchWithoutLock(std::shared_ptr<Core::Item> &&item, uint score);

    Query() = default;
    ~Query() = default;

    std::vector<std::pair<std::shared_ptr<Item>, uint>> results_;
    QMutex mutex_;
    QString trigger_;
    QString string_;
    QString rawString_;
    bool isValid_ = true;
    std::map<QString, uint> scores_;

    friend class QueryExecution;
};

}


