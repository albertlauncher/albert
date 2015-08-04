// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#include "prefixsearch.h"

template<class T>
class FuzzySearch final : public PrefixSearch<T> {
public:

    /** ***********************************************************************/
    explicit FuzzySearch(unsigned int q = 3, double d = 2) : _q(q), _delta(d){
    }



    /** ***********************************************************************/
    explicit FuzzySearch(const PrefixSearch<T>& rhs, unsigned int q = 3, double d = 2) : PrefixSearch<T>(rhs), _q(q), _delta(d) {
        // Iterate over the inverted index and build the qGramindex
        for (typename PrefixSearch<T>::InvertedIndex::const_iterator it = this->_invertedIndex.constBegin(); it != this->_invertedIndex.constEnd(); ++it){
            QString spaced = QString(_q-1,' ').append(it.key());
            for (unsigned int i = 0 ; i < static_cast<unsigned int>(it.key().size()); ++i)
                ++_qGramIndex[spaced.mid(i,_q)][it.key()];
        }
    }



    /** ***********************************************************************/
    ~FuzzySearch(){
    }



    /** ***********************************************************************/
    void build(const QList<T>& lso) override {
        for (T obj : lso){
            // Add a mappings to the inverted index which maps on t.
            QStringList aliases = obj->alises() << obj->name();
            for (const QString & str : aliases) {
                QStringList words = str.split(QRegularExpression(SEPARATOR_REGEX), QString::SkipEmptyParts);
                 for (QString& w : words) {

                    // Make this search case insensitive
                    w.toLower();

                    // Add word to inverted index (map word to item)
                    this->_invertedIndex[w].insert(obj);

                    // Build a qGram index (map substring to word)
                    QString spaced = QString(_q-1,' ').append(w);
                    for (unsigned int i = 0 ; i < static_cast<unsigned int>(w.size()); ++i)
                        ++_qGramIndex[spaced.mid(i,_q)][w]; //FIXME Currently occurences are not uses
                }
            }
        }
    }


    /** ***********************************************************************/
    void clear() override
    {
        this->_invertedIndex.clear();
        _qGramIndex.clear();
    }



    /** ***********************************************************************/
    QList<SharedObject> search(const QString &req) const override
    {
        QVector<QString> words;
        for (QString &word : req.split(SEPARATOR_REGEX, QString::SkipEmptyParts))
            words.append(word.toLower());
        QVector<QMap<SharedObject, unsigned int>> resultsPerWord;

        // Quit if there are no words in query
        if (words.empty()) return QList<SharedObject>();

        // Split the query into words
        for (QString &word : words)
        {
            unsigned int delta = static_cast<unsigned int>((_delta < 1)? word.size()/_delta : _delta);

            // Generate the qGrams of this word
            QMap<QString, unsigned int> qGrams;
            QString spaced(_q-1,' ');
            spaced.append(word.toLower());
            for (unsigned int i = 0 ; i < static_cast<unsigned int>(word.size()); ++i)
                ++qGrams[spaced.mid(i,_q)];

            // Get the words referenced by each qGram an increment their
            // reference counter
            // Iterate over the set of qgrams in the word
            QMap<QString, unsigned int> wordMatches;
            for (QMap<QString, unsigned int>::const_iterator it = qGrams.cbegin(); it != qGrams.end(); ++it)
            {
                // Iterate over the set of words referenced by this qGram
                for (QMap<QString, unsigned int>::const_iterator wit = _qGramIndex[it.key()].begin(); wit != _qGramIndex[it.key()].cend(); ++wit)
                {
                    // CRUCIAL: The match can contain only the commom amount of qGrams
                    wordMatches[wit.key()] += (it.value() < wit.value()) ? it.value() : wit.value();
                }
            }

            // Allocate a new set
            resultsPerWord.push_back(QMap<SharedObject, unsigned int>());
            QMap<SharedObject, unsigned int>& resultsRef = resultsPerWord.back();

            // Unite the items referenced by the words accumulating their #matches
            for (QMap<QString, unsigned int>::const_iterator wm = wordMatches.begin(); wm != wordMatches.cend(); ++wm)
            {
                //			// Do some kind of (cheap) preselection by mathematical bound
                //			if (wm.value() < qGrams.size()-delta*_q)
                //				continue;

                // Now check the (expensive) prefix edit distance
                if (!checkPrefixEditDistance(word, wm.key(), delta))
                    continue;


                for(SharedObject item : this->_invertedIndex[wm.key()])
                {
                    resultsRef[item] += wm.value();
                }
            }
        }

        // Intersect the set of items references by the (referenced) words
        // This assusmes that there is at least one word (the query would not have
        // been started elsewise)
        QVector<QPair<SharedObject, unsigned int>> finalResult;
        if (resultsPerWord.size() > 1)
        {
            // Get the smallest list for intersection (performance)
            unsigned int smallest=0;
            for (unsigned int i = 1; i < static_cast<unsigned int>(resultsPerWord.size()); ++i)
                if (resultsPerWord[i].size() < resultsPerWord[smallest].size())
                    smallest = i;

            bool allResultsContainEntry;
            for (QMap<SharedObject, unsigned int>::const_iterator r = resultsPerWord[smallest].begin(); r != resultsPerWord[smallest].cend(); ++r)
            {
                // Check if all results contain this entry
                allResultsContainEntry=true;
                unsigned int accMatches = resultsPerWord[smallest][r.key()];
                for (unsigned int i = 0; i < static_cast<unsigned int>(resultsPerWord.size()); ++i)
                {
                    // Ignore itself
                    if (i==smallest)
                        continue;

                    // If it is in: check next relutlist
                    if (resultsPerWord[i].contains(r.key()))
                    {
                        // Accumulate matches
                        accMatches += resultsPerWord[i][r.key()];
                        continue;
                    }

                    allResultsContainEntry = false;
                    break;
                }

                // If this is not common, check the next entry
                if (!allResultsContainEntry)
                    continue;

                // Finally this match is common an can be put into the results
                finalResult.append(QPair<SharedObject, unsigned int>(r.key(), accMatches));
            }
        }
        else // Else do it without intersction
        {
            for (QMap<SharedObject, unsigned int>::const_iterator r = resultsPerWord[0].begin(); r != resultsPerWord[0].cend(); ++r)
                finalResult.append(QPair<SharedObject, unsigned int>(r.key(), r.value()));
        }

        // Sort em by relevance // TODO INTRODUCE RELEVANCE TO ITEMS
        //        std::sort(finalResult.begin(), finalResult.end(),
        //                  [&](QPair<T, unsigned int> x, QPair<T, unsigned int> y)
        //                    {return x.second > y.second;});
        QList<SharedObject> result;
        for (QPair<SharedObject, unsigned int> pair : finalResult){
            result << pair.first;
        }
        return result;
    }


    /** ***********************************************************************/
	inline double delta() const {return _delta;}
	inline void setDelta(double d){_delta=d;}

private:
    /** ***********************************************************************/
    static bool checkPrefixEditDistance(const QString& prefix, const QString& str, unsigned int delta)
    {
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
            if (matrix[(n-1)+j] <= delta) {
                //		qDebug() << prefix << "~" << str << matrix[n - 1][j];
                result = true;
                break;
            }
        }
        delete matrix;
        return result;
    }

    /** ***********************************************************************/
    // Map of qGrams, containing their word references and #occurences
    typedef QMap<QString, QMap<QString, unsigned int>> QGramIndex;
	QGramIndex _qGramIndex;

    unsigned int _q; // Size of the slices
    double _delta; // Maximum error
};
