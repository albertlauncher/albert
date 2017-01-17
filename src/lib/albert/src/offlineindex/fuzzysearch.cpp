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
using std::shared_ptr;
using std::vector;

namespace {

bool checkPrefixEditDistance(const QString &prefix, const QString &str, unsigned int delta) {
    bool result = false;
    unsigned int n = prefix.size() + 1;
    unsigned int m = std::min(prefix.size() + delta + 1, static_cast<unsigned int>(str.size()) + 1);
    //      unsigned int matrix[n][m];
    // This MAY throw an error
    unsigned int* matrix = new unsigned int[n*m];

    // Initialize left and top row.
    //      for (unsigned int i = 0; i < n; ++i) { matrix[i][0] = i; }
    //      for (unsigned int i = 0; i < m; ++i) { matrix[0][i] = i; }
    for (unsigned int i = 0; i < n; ++i) { matrix[i*m+0] = i; }
    for (unsigned int i = 0; i < m; ++i) { matrix[0*m+i] = i; }

    // Now fill the whole matrix.
    for (unsigned int i = 1; i < n; ++i) {
        for (unsigned int j = 1; j < m; ++j) {
            unsigned int dia = matrix[(i-1)*m+j-1] + (prefix[i-1] == str[j-1] ? 0 : 1);
            matrix[i*m+j] = std::min(std::min(
                                         dia,
                                         matrix[i*m+j-1] + 1),
                    matrix[(i-1)*m+j] + 1);
        }
    }
    // Check the last row if there is an entry <= delta.
    for (unsigned int j = 0; j < m; ++j) {
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
Core::FuzzySearch::FuzzySearch(unsigned int q, double d) : q_(q), delta_(d) {

}



/** ***************************************************************************/
Core::FuzzySearch::FuzzySearch(const Core::PrefixSearch &rhs, unsigned int q, double d) : PrefixSearch(rhs), q_(q), delta_(d) {
    // Iterate over the inverted index and build the qGramindex
    for (typename PrefixSearch::InvertedIndex::const_iterator it = this->invertedIndex_.cbegin(); it != this->invertedIndex_.cend(); ++it) {
        QString spaced = QString(q_-1,' ').append(it->first);
        for (unsigned int i = 0 ; i < static_cast<unsigned int>(it->first.size()); ++i)
            ++qGramIndex_[spaced.mid(i,q_)][it->first];
    }
}



/** ***************************************************************************/
Core::FuzzySearch::~FuzzySearch() {

}



/** ***************************************************************************/
void Core::FuzzySearch::add(shared_ptr<Core::Indexable> idxble) {
    // Add a mappings to the inverted index which maps on t.
    std::vector<Indexable::WeightedKeyword> indexKeywords = idxble->indexKeywords();
    for (const auto &wkw : indexKeywords) {
        QStringList words = wkw.keyword.split(QRegularExpression(SEPARATOR_REGEX), QString::SkipEmptyParts);
        for (QString &w : words) {

            // Make this search case insensitive
            w=w.toLower();

            // Add word to inverted index (map word to item)
            this->invertedIndex_[w].insert(idxble);

            // Build a qGram index (map substring to word)
            QString spaced = QString(q_-1,' ').append(w);
            for (unsigned int i = 0 ; i < static_cast<unsigned int>(w.size()); ++i)
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
    vector<map<shared_ptr<Indexable>, unsigned int>> resultsPerWord;

    // Quit if there are no words in query
    if (words.empty())
        return vector<shared_ptr<Indexable>>();

    // Split the query into words
    for (QString &word : words) {
        unsigned int delta = static_cast<unsigned int>((delta_ < 1)? word.size()/delta_ : delta_);

        // Generate the qGrams of this word
        map<QString, unsigned int> qGrams;
        QString spaced(q_-1,' ');
        spaced.append(word.toLower());
        for (unsigned int i = 0 ; i < static_cast<unsigned int>(word.size()); ++i)
            ++qGrams[spaced.mid(i,q_)];

        // Get the words referenced by each qGram an increment their
        // reference counter
        // Iterate over the set of qgrams in the word
        map<QString, unsigned int> wordMatches;
        for (map<QString, unsigned int>::const_iterator it = qGrams.cbegin(); it != qGrams.end(); ++it) {

            // Check for existance (std::map::at throws)
            if (qGramIndex_.count(it->first) == 0)
                continue;

            // Iterate over the set of words referenced by this qGram
            for (map<QString, unsigned int>::const_iterator wit = qGramIndex_.at(it->first).cbegin();
                 wit != qGramIndex_.at(it->first).cend(); ++wit) {
                // CRUCIAL: The match can contain only the commom amount of qGrams
                wordMatches[wit->first] += (it->second < wit->second) ? it->second : wit->second;
            }
        }

        // Allocate a new set
        resultsPerWord.push_back(map<shared_ptr<Indexable>, unsigned int>());
        map<shared_ptr<Indexable>, unsigned int>& resultsRef = resultsPerWord.back();

        // Unite the items referenced by the words accumulating their #matches
        for (map<QString, unsigned int>::const_iterator wm = wordMatches.begin(); wm != wordMatches.end(); ++wm) {
            //			// Do some kind of (cheap) preselection by mathematical bound
            //			if (wm.value() < qGrams.size()-delta*_q)
            //				continue;

            // Now check the (expensive) prefix edit distance
            if (!checkPrefixEditDistance(word, wm->first, delta))
                continue;

            // Check for existance (std::map::at throws)
            if (invertedIndex_.count(wm->first) == 0)
                continue;

            for(const shared_ptr<Indexable> &item : invertedIndex_.at(wm->first)) {
                resultsRef[item] += wm->second;
            }
        }
    }

    // Intersect the set of items references by the (referenced) words
    // This assusmes that there is at least one word (the query would not have
    // been started elsewise)
    vector<std::pair<shared_ptr<Indexable>, unsigned int>> finalResult;
    if (resultsPerWord.size() > 1) {
        // Get the smallest list for intersection (performance)
        unsigned int smallest=0;
        for (unsigned int i = 1; i < static_cast<unsigned int>(resultsPerWord.size()); ++i)
            if (resultsPerWord[i].size() < resultsPerWord[smallest].size())
                smallest = i;

        bool allResultsContainEntry;
        for (map<shared_ptr<Indexable>, unsigned int>::const_iterator r = resultsPerWord[smallest].begin();
             r != resultsPerWord[smallest].cend(); ++r) {
            // Check if all results contain this entry
            allResultsContainEntry=true;
            unsigned int accMatches = resultsPerWord[smallest][r->first];
            for (unsigned int i = 0; i < static_cast<unsigned int>(resultsPerWord.size()); ++i) {
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
        for (map<shared_ptr<Indexable>, unsigned int>::const_iterator r = resultsPerWord[0].begin();
             r != resultsPerWord[0].cend(); ++r)
            finalResult.push_back(std::make_pair(r->first, r->second));
    }

    // Sort em by relevance // TODO INTRODUCE RELEVANCE TO ITEMS
    //        std::sort(finalResult.begin(), finalResult.end(),
    //                  [](QPair<T, unsigned int> x, QPair<T, unsigned int> y)
    //                    {return x.second > y.second;});
    vector<shared_ptr<Indexable>> result;
    for (const std::pair<shared_ptr<Indexable>, unsigned int> &pair : finalResult) {
        result.push_back(pair.first);
    }
    return result;
}


