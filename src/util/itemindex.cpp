// Copyright (c) 2021-2024 Manuel Schneider

#include "item.h"
#include "itemindex.h"
#include "levenshtein.h"
#include "logging.h"
#include <QRegularExpression>
#include <algorithm>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
using namespace albert;
using namespace std;

namespace
{

using Index = uint32_t;
using Position = uint16_t;
static const uint N = 2;


struct StringIndexItem
{
    uint32_t item_index;
    uint16_t max_match_len;
};


struct Location
{
    Index index;
    Position position;
};


struct WordIndexItem
{
    QString word;
    vector<Location> occurrences;
    // TODO: heres the place to add a term_frequency weighting approach
};


struct WordMatch
{
    const WordIndexItem &word_index_item;
    uint match_length;
};


struct StringMatch
{
    Index index;
    Position position;
    uint16_t match_len;
};


struct IndexData
{
    ///
    /// The flat random access index of unique items.
    ///
    vector<shared_ptr<albert::Item>> items;

    ///
    /// The string index (Inverted item index).
    ///
    /// Multiple strings can point to items.
    /// Technically the string itself is not needed/stored.
    /// The information is kept in the word index though.
    ///
    /// s_idx > (i_idx, mml)
    ///
    vector<StringIndexItem> strings;

    ///
    /// The word index (inverted string index).
    ///
    /// Holds pointers to word occurrences.
    ///
    /// w_idx > (word, [ (s_idx, w_pos) ] )
    ///
    vector<WordIndexItem> words;

    ///
    /// The nGram index.
    ///
    unordered_map<QString, vector<Location>> ngrams;
};

}

class ItemIndex::Private
{
public:
    MatchConfig config;
    mutable shared_mutex mutex;
    IndexData index;

    QStringList tokenize(QString string) const;
    vector<QString> ngrams_for_word(const QString &word)const;
    vector<WordMatch> getWordMatches(const QString &word, const bool &isValid) const;
    vector<StringMatch> getStringMatches(const QString &word, const bool &isValid) const;
};

QStringList ItemIndex::Private::tokenize(QString s) const
{
    // Remove soft hyphens
    s.remove(QChar(0x00AD));

    if (config.ignore_diacritics)
    {
        // https://en.wikipedia.org/wiki/Combining_Diacritical_Marks
        static QRegularExpression re(R"([\x{0300}-\x{036f}])");
        s = s.normalized(QString::NormalizationForm_D).remove(re);
    }

    if (config.ignore_case)
        s = s.toLower();

    auto t = s.split(config.separator_regex, Qt::SkipEmptyParts);

    if (config.ignore_word_order)
        t.sort();

    return t;
}

vector<QString> ItemIndex::Private::ngrams_for_word(const QString &word) const
{
    vector<QString> ngrams;
    ngrams.reserve(word.size());
    auto padded = QString("%1%2").arg(QString(N - 1, ' '), word);
    for (int i = 0; i < word.size(); ++i){
        QString ngram{padded.mid(i, N)};
        ngram.shrink_to_fit();
        ngrams.emplace_back(ngram);
    }
    return ngrams;
}

vector<WordMatch> ItemIndex::Private::getWordMatches(const QString &word, const bool &isValid) const
{
    vector<WordMatch> matches;
    const uint word_length = word.length();

    // Get range of perfect prefix match words
    const auto &[eq_begin, eq_end] =
            equal_range(
                index.words.cbegin(), index.words.cend(), WordIndexItem{word, {}},
                [l=word_length](const WordIndexItem &a, const WordIndexItem &b)
                { return QStringView{a.word}.left(l) < QStringView{b.word}.left(l); }
            );

    // Store perfect prefix match words
    for (auto it = eq_begin; it != eq_end; ++it)
        matches.emplace_back(*it, word_length);

    // Get the (fuzzy) prefix matches
    if (config.fuzzy)
    {
        // Exclusion range for already collected prefix matches
        Index exclude_begin = eq_begin - index.words.begin();  // Ignore interval. closed begin [
        Index exclude_end = eq_end - index.words.begin();  // Ignore interval. open end )

        auto ngrams = ngrams_for_word(word);

        // Get the words referenced by each nGram
        unordered_map<Index, uint> word_match_counts;
        for (const QString &n_gram : ngrams)
        {
            if (!isValid)
                return {};

            // Get the ngram occurrences
            if (auto it = index.ngrams.find(n_gram); it != index.ngrams.end())
            {
                // Iterate all ngram occurrences
                for (const auto &ngram_occurrences : it->second)
                {
                    // Excluding the existing perfect matches
                    if (exclude_begin <= ngram_occurrences.index
                        && ngram_occurrences.index < exclude_end)
                        continue;

                    // count the ngrams where position < word_length
                    if (ngram_occurrences.position < static_cast<Position>(word_length))
                        ++word_match_counts[ngram_occurrences.index];
                }
            }
        }

        // First do a cheap preselection by mathematical bound.
        // Then compute the edit distance to filter matches.
        // If there are less than |word_length|-δ*n matching qGrams it is no
        // match. If the common qGrams are less than |word|-δ*q this implies
        // that there are more errors than δ.

        Levenshtein levenshtein;
        uint allowed_errors = word_length / config.error_tolerance_divisor;
        uint minimum_match_count = word_length - allowed_errors * N;

        for (const auto &[word_idx, ngram_count]: word_match_counts)
        {
            if (!isValid)
                return {};

            if (ngram_count < minimum_match_count)
                continue;

            if (auto edit_distance =
                    levenshtein.computePrefixEditDistanceWithLimit(
                        word, index.words[word_idx].word, allowed_errors);
                    edit_distance > allowed_errors)
                continue;
            else
                matches.emplace_back(index.words[word_idx], word_length-edit_distance);
        }
    }

    return matches;
}

vector<StringMatch>
ItemIndex::Private::getStringMatches(const QString &word, const bool &isValid) const
{
    vector<StringMatch> string_matches;

    for (const auto &word_match : getWordMatches(word, isValid))
        for (const auto &occurrence : word_match.word_index_item.occurrences)
            string_matches.emplace_back(occurrence.index, occurrence.position, word_match.match_length);

    sort(string_matches.begin(), string_matches.end(),
         [](auto &l, auto &r){ return l.index < r.index; });

    return string_matches;
}


ItemIndex::ItemIndex(MatchConfig config)
    : d(new Private{.config = ::move(config), .mutex = {}, .index = {}}) {}

ItemIndex &ItemIndex::operator=(ItemIndex &&) = default;

ItemIndex::ItemIndex(ItemIndex &&) = default;

ItemIndex::~ItemIndex() = default;

const MatchConfig &ItemIndex::config() { return d->config; }

void ItemIndex::setItems(vector<albert::IndexItem> &&index_items)
{
    IndexData new_index;

    unordered_map<albert::Item*,Index> item_indices_;  // implicit unique
    map<QString,WordIndexItem> word_index_;  // implicit lexicographical order

    for (auto &[item, string] : index_items)
    {
        QStringList &&words = d->tokenize(string);
        if (words.empty())
        {
            WARN << QString("Skipping index entry '%1'. Tokenization of '%2' yields empty set.")
                        .arg(item->id(), string);
            continue;
        }

        // Try to add the item to the temporary item index map (ensures uniqueness)
        // Assume it is going to be added to the end
        const auto &[it, emplaced] = item_indices_.emplace(item.get(), (Index)new_index.items.size());

        // If item does not exist, move it into the index.
        if (emplaced)
            new_index.items.emplace_back(::move(item));

        // Add string to item mapping.
        auto &string_index_item = new_index.strings.emplace_back(it->second, 0);

        // Iterate the words
        for (Position p = 0; p < (Position)words.size(); ++p)
        {
            // Add word to string mapping.
            word_index_[words[p]].occurrences.emplace_back(new_index.strings.size() - 1, p);

            // Store the maximal match length for scoring
            string_index_item.max_match_len += words[p].size();
        }
    }

    new_index.items.shrink_to_fit();
    new_index.strings.shrink_to_fit();

    // Build the random access word index
    for (auto &[word, word_index_item] : word_index_)
    {
        word_index_item.word = word;
        word_index_item.word.shrink_to_fit();
        word_index_item.occurrences.shrink_to_fit();
        new_index.words.emplace_back(::move(word_index_item));
    }
    new_index.words.shrink_to_fit();

    if (d->config.fuzzy)
    {
        // Build n_gram_index
        for (Index word_index = 0; word_index < (Index)new_index.words.size(); ++word_index)
        {
            auto ngrams = d->ngrams_for_word(new_index.words[word_index].word);
            for (Position pos = 0 ; pos < (Position)ngrams.size(); ++pos)
                new_index.ngrams[ngrams[pos]].emplace_back(word_index, pos);
        }
    }
    for (auto &[_, word_refs] : new_index.ngrams)
        word_refs.shrink_to_fit();

    unique_lock lock(d->mutex);
    d->index = new_index;
}

vector<albert::RankItem> ItemIndex::search(const QString &string, const bool &isValid) const
{
    QStringList &&words = d->tokenize(string);

    unordered_map<Index, double> result_map;

    if (words.empty())

        // Return all items
        for (const auto &string_index_item : d->index.strings)
            result_map.emplace(string_index_item.item_index, 0.0f);

    else
    {
        shared_lock lock(d->mutex);


        vector<StringMatch> string_matches = d->getStringMatches(words[0], isValid);

        // In case of multiple words intersect. Todo: user chooses strategy
        for (int w = 1; w < words.size(); ++w)
        {
            if (!isValid || string_matches.empty())
                return {};

            vector<StringMatch> other_string_matches = d->getStringMatches(words[w], isValid);

            if (other_string_matches.empty())
                return {};

            vector<StringMatch> new_string_matches;
            for (auto lit = string_matches.cbegin(); lit != string_matches.cend();)
            {
                // Build a range of upcoming left_matches with same index
                auto elit = lit;
                while(elit != string_matches.cend() && lit->index==elit->index)
                    ++elit;

                // Get the range of equal string matches on the right side
                const auto &[eq_begin, eq_end] =
                        equal_range(other_string_matches.cbegin(), other_string_matches.cend(),
                                    *lit, [](auto &l, auto &r) { return l.index < r.index; });

                // If no match on the right side continue with next leftmatch
                if (eq_begin == eq_end){
                    lit = elit;
                    continue;
                }

                // Intersect and aggregate match lengths
                for (;lit != elit; ++lit)
                    for (auto rit = eq_begin; rit != eq_end; ++rit)
                        if (lit->position < rit->position)  // Sequence check
                            new_string_matches.emplace_back(rit->index, rit->position,
                                                            rit->match_len + lit->match_len);
            }

            string_matches = ::move(new_string_matches);
        }

        // Build the list of matched items with their highest scoring match
        for (const auto &match : string_matches)
        {
            double score = (double)match.match_len / d->index.strings[match.index].max_match_len;

            const auto &[it, success] =
                    result_map.emplace(d->index.strings[match.index].item_index, score);

            // Update score if exists and is less
            if (!success && it->second < score)
                it->second = score;
        }

    }

    // Convert results to return type
    vector<RankItem> result;
    result.reserve(result_map.size());
    for (const auto &[item_idx, score] : result_map)
        result.emplace_back(d->index.items[item_idx], score);

    return result;
}
