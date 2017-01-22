// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include "fuzzysearch.h"
#include "indexable.h"
#include "prefixsearch.h"
using std::map;
using std::set;
using std::pair;
using std::shared_ptr;
using std::vector;

namespace {

bool checkPrefixEditDistance(const QString &prefix, const QString &str, uint delta) {
    uint n = prefix.size() + 1;
    uint m = std::min(prefix.size() + delta + 1, static_cast<uint>(str.size()) + 1);

    uint* matrix = new uint[n*m];

    // Initialize left and top row.
    for (uint i = 0; i < n; ++i) { matrix[i*m+0] = i; }
    for (uint i = 0; i < m; ++i) { matrix[0*m+i] = i; }

    // Now fill the whole matrix.
    for (uint i = 1; i < n; ++i) {
        for (uint j = 1; j < m; ++j) {
            uint dia = matrix[(i-1)*m+j-1] + (prefix[i-1] == str[j-1] ? 0 : 1);
            matrix[i*m+j] = std::min(std::min(
                                         dia,
                                         matrix[i*m+j-1] + 1),
                    matrix[(i-1)*m+j] + 1);
        }
    }

    // Check the last row if there is an entry <= delta.
    bool result = false;
    for (uint j = 0; j < m; ++j) {
        if (matrix[(n-1)*m+j] <= delta) {
            result = true;
            break;
        }
    }
    delete[] matrix;
    return result;
}

}



/** ***************************************************************************/
Core::FuzzySearch::FuzzySearch(uint q, double d) : q_(q), delta_(d) {

}



/** ***************************************************************************/
Core::FuzzySearch::FuzzySearch(const Core::PrefixSearch &rhs, uint q, double d) : PrefixSearch(rhs), q_(q), delta_(d) {
    // Iterate over the inverted index and build the qGramindex
    for (typename PrefixSearch::InvertedIndex::const_iterator it = this->invertedIndex_.cbegin(); it != this->invertedIndex_.cend(); ++it) {
        QString spaced = QString(q_-1,' ').append(it->first);
        for (uint i = 0 ; i < static_cast<uint>(it->first.size()); ++i)
            ++qGramIndex_[spaced.mid(i,q_)][it->first];
    }
}



/** ***************************************************************************/
Core::FuzzySearch::~FuzzySearch() {

}



/** ***************************************************************************/
void Core::FuzzySearch::add(shared_ptr<Core::Indexable> indexable) {

    // Add indexable to the index
    index_.push_back(indexable);
    uint id = static_cast<uint>(index_.size()-1);

    // Add a mappings to the inverted index which maps on t.
    vector<Indexable::WeightedKeyword> indexKeywords = indexable->indexKeywords();
    for (const auto &wkw : indexKeywords) {
        QStringList words = wkw.keyword.split(QRegularExpression(SEPARATOR_REGEX), QString::SkipEmptyParts);
        for (QString &w : words) {

            // Make this search case insensitive
            w=w.toLower();

            // Add word to inverted index (map word to item)
            this->invertedIndex_[w].insert(id);

            // Build a qGram index (map substring to word)
            QString spaced = QString(q_-1,' ').append(w);
            for (uint i = 0 ; i < static_cast<uint>(w.size()); ++i)
                ++qGramIndex_[spaced.mid(i,q_)][w]; //FIXME Currently occurences are not uses
        }
    }
}



/** ***************************************************************************/
void Core::FuzzySearch::clear() {
    this->invertedIndex_.clear();
    qGramIndex_.clear();
}



/** ***************************************************************************/
vector<shared_ptr<Core::Indexable> > Core::FuzzySearch::search(const QString &req) const {
    vector<QString> words;
    for (QString &word : req.split(QRegularExpression(SEPARATOR_REGEX), QString::SkipEmptyParts))
        words.push_back(word.toLower());
    vector<map<uint,uint>> resultsPerWord; // id, count

    // Quit if there are no words in query
    if (words.empty())
        return vector<shared_ptr<Indexable>>();

    // Split the query into words
    for (QString &word : words) {

        uint delta = static_cast<uint>((delta_ < 1)? word.size()*delta_ : delta_);

        // Generate the qGrams of this word
        map<QString,uint> qGrams;
        QString spaced(q_-1,' ');
        spaced.append(word.toLower());
        for ( uint i = 0; i < static_cast<uint>(word.size()); ++i )
            ++qGrams[spaced.mid(i,q_)];

        // Get the words referenced by each qGram and count the references
        map<QString,uint> wordMatches;
        for ( const pair<QString,uint> &qGram : qGrams) {

            // Find the qGram in the index, skip if nothing found
            decltype(qGramIndex_)::const_iterator qGramIndexIt = qGramIndex_.find(qGram.first);
            if ( qGramIndexIt == qGramIndex_.end() )
                continue;

            // Iterate over the set of words referenced by this qGram
            for (const pair<QString,uint> &indexEntry : qGramIndexIt->second) {
                // CRUCIAL: The match can contain only the commom amount of qGrams
                wordMatches[indexEntry.first] += std::min(qGram.second, indexEntry.second);
            }
        }

        // Unite the items referenced by the words accumulating their #matches
        map<uint,uint> results; // id, count
        for (const pair<QString,uint> &wordMatch : wordMatches) {

            /*
             * Do some kind of (cheap) preselection by mathematical bound
             * If the matched word has less than |word|-δ*q matching qGrams
             * it cannot be a match.
             * This is because a single error can reduce the common qGram by
             * maximum q. δ errors can therefore reduce the common qGrams by
             * maximum δ*q. If the common qGrams are less than |word|-δ*q this
             * implies that there are more errors than δ.
             */
            if (wordMatch.second < (word.size()-delta*q_) )
                continue;

            // Now check the (expensive) prefix edit distance
            if (!checkPrefixEditDistance(word, wordMatch.first, delta))
                continue;

            // Checks should not be neccessary since this builds on the index
            for(uint id : invertedIndex_.at(wordMatch.first)) {
                results[id] += wordMatch.second;
            }
        }

        resultsPerWord.push_back(std::move(results));
    }

    // Intersect the set of items references by the (referenced) words
    // This assusmes that there is at least one word (the query would not have
    // been started elsewise)
    vector<pair<uint,uint>> finalResult;
    if (resultsPerWord.size() > 1) {
        // Get the smallest list for intersection (performance)
        uint smallest=0;
        for (uint i = 1; i < static_cast<uint>(resultsPerWord.size()); ++i)
            if (resultsPerWord[i].size() < resultsPerWord[smallest].size())
                smallest = i;

        bool allResultsContainEntry;
        for (map<uint,uint>::const_iterator r = resultsPerWord[smallest].begin();
             r != resultsPerWord[smallest].cend(); ++r) {
            // Check if all results contain this entry
            allResultsContainEntry=true;
            uint accMatches = resultsPerWord[smallest][r->first];
            for (uint i = 0; i < static_cast<uint>(resultsPerWord.size()); ++i) {
                // Ignore itself
                if (i==smallest)
                    continue;

                // If it is in: check next relutlist
                if (resultsPerWord[i].find(r->first) != resultsPerWord[i].end() ) {
                    // Accumulate matches
                    accMatches += resultsPerWord[i][r->first];
                    continue;
                }

                allResultsContainEntry = false;
                break;
            }

            // If this is not common, check the next entry
            if (!allResultsContainEntry)
                continue;

            // Finally this match is common an can be put into the results
            finalResult.push_back(std::make_pair(r->first, accMatches));
        }
    } else {// Else do it without intersction
        for ( const pair<uint,uint> &result : resultsPerWord[0] )
            finalResult.push_back(std::make_pair(result.first, result.second));
    }

    // Sort em by relevance // TODO INTRODUCE RELEVANCE TO ITEMS
    //        std::sort(finalResult.begin(), finalResult.end(),
    //                  [](QPair<T, uint> x, QPair<T, uint> y)
    //                    {return x.second > y.second;});
    vector<shared_ptr<Indexable>> result;
    for (const pair<uint,uint> &pair : finalResult) {
        result.push_back(index_.at(pair.first));
    }
    return result;
}


