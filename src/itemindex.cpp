// Copyright (c) 2021-2023 Manuel Schneider

#include "albert/extensions/queryhandler.h"
#include "itemindex.h"
#include "levenshtein.h"
#include <QRegularExpression>
#include <map>
#include <algorithm>
#include <utility>
#include <mutex>
using namespace std;
using namespace albert;
using Score = RankItem::Score;


static QStringList splitString(const QString &string, const QString &separators, bool case_sensitive = false)
{
    return ((!case_sensitive) ? string.toLower(): string).split(QRegularExpression(separators), Qt::SkipEmptyParts);
}

static vector<QString> ngrams_for_word(const QString &word, uint n)
{
    vector<QString> ngrams;
    ngrams.reserve(word.size());
    auto padded = QString("%1%2").arg(QString(n - 1, ' '), word);
    for (int i = 0; i < word.size(); ++i){
        QString ngram{padded.mid(i, n)};
        ngram.shrink_to_fit();
        ngrams.emplace_back(ngram);
    }
    return ngrams;
}

ItemIndex::ItemIndex(QString sep, bool cs, uint n_, uint etd)
    : case_sensitive(cs), error_tolerance_divisor(etd), separators(std::move(sep)), n(n_)
{
}

void ItemIndex::setItems(std::vector<albert::IndexItem> &&index_items)
{
    IndexData index_;

    unordered_map<albert::Item*,Index> item_indices_;  // implicit unique
    map<QString,WordIndexItem> word_index_;  // implicit lexicographical order

    for (Index string_index = 0; string_index < (Index)index_items.size(); ++string_index) {

        // Add item to the unique item index
        auto item_index = (Index)index_.items.size();
        auto [it, emplaced] = item_indices_.emplace(index_items[string_index].item.get(), item_index);
        if (emplaced)
            index_.items.emplace_back(::move(index_items[string_index].item));
        else
            item_index = it->second;

        // Add a string index entry for each string. Store the maximal match length for scoring
        QStringList &&words = splitString(index_items[string_index].string, separators, case_sensitive);
        uint max_match_len = 0;
        for (const auto& word : words)
            max_match_len += word.size();
        index_.strings.emplace_back(item_index, max_match_len);

        // Add this string to the occurences in the word index.
        for (Position pos = 0; pos < (Position)words.size(); ++pos)
            word_index_[words[pos]].occurrences.emplace_back(string_index, pos);
    }
    index_.items.shrink_to_fit();
    index_.strings.shrink_to_fit();

    // Build the random access word index
    for (auto &[word, word_index_item] : word_index_) {
        word_index_item.word = word;
        word_index_item.word.shrink_to_fit();
        word_index_item.occurrences.shrink_to_fit();
        index_.words.emplace_back(::move(word_index_item));
    }
    index_.words.shrink_to_fit();

    if (error_tolerance_divisor){
        // build q_gram_index
        for (Index word_index = 0; word_index < (Index)index_.words.size(); ++word_index) {
            vector<QString> ngrams(ngrams_for_word(index_.words[word_index].word, n));
            for (Position pos = 0 ; pos < (Position)ngrams.size(); ++pos)
                index_.ngrams[ngrams[pos]].emplace_back(word_index, pos);
        }
    }
    for (auto &[_, word_refs] : index_.ngrams)
        word_refs.shrink_to_fit();

    unique_lock lock(mutex);
    index = index_;
}

std::vector<ItemIndex::WordMatch> ItemIndex::getWordMatches(const QString &word, const bool &isValid) const
{
    vector<WordMatch> matches;
    const uint word_length = word.length();

    // Get range of perfect prefix match words
    const auto &[eq_begin, eq_end] = equal_range(index.words.cbegin(), index.words.cend(), WordIndexItem{word, {}},
                                                 [l=word_length](const WordIndexItem &a, const WordIndexItem &b) {
                                                     return QStringView{a.word}.left(l) < QStringView{b.word}.left(l);
                                                 });

    // Store perfect prefix match words
    for (auto it = eq_begin; it != eq_end; ++it)
        matches.emplace_back(*it, word_length);

    // Get the (fuzzy) prefix matches
    if (error_tolerance_divisor) {
        Index prefix_match_first_id = eq_begin - index.words.begin();  // Ignore interval. closed begin [
        Index prefix_match_last_id = eq_end - index.words.begin();  // Ignore interval. open end )

        // Get the words referenced by each nGram and count the ngrams where position < word_length.
        vector<QString> ngrams(ngrams_for_word(word, n));
        unordered_map<Index,uint> word_match_counts;

        for (const QString &n_gram: ngrams) {
            try {
                for (const auto &ngram_occ: index.ngrams.at(n_gram)) {
                    // Exclude the existing perfect matches
                    if (prefix_match_first_id <= ngram_occ.index && ngram_occ.index < prefix_match_last_id)
                        continue;

                    if (ngram_occ.position < static_cast<Position>(word_length))
                        ++word_match_counts[ngram_occ.index];
//                    else
//                        break;  // wtf is this
                }
            }
            catch (const out_of_range &)
            {
                // NOTE room for optimizations?
            }
        }

        // Get the words referenced by the grams, filter by bound, compute edit distance, add match
        // Do (cheap) preselection by mathematical bound. If there are less than |word_length|-δ*n matching qGrams
        // it is no match. If the common qGrams are less than |word|-δ*q this implies that there are more errors
        // than δ.
        uint allowed_errors = (uint)((float)word_length/(float)error_tolerance_divisor);
        uint minimum_match_count = word_length - allowed_errors * n;
        Levenshtein levenshtein;
        for (const auto &[word_idx, ngram_count]: word_match_counts) {
            if (ngram_count < minimum_match_count || !isValid)
                continue;

            if (auto edit_distance = levenshtein.computePrefixEditDistanceWithLimit(word, index.words[word_idx].word,
                                                                                    allowed_errors);
                    edit_distance > allowed_errors)
                continue;
            else
                matches.emplace_back(index.words[word_idx], word_length-edit_distance);
        }
    }
    return matches;
}

std::vector<albert::RankItem> ItemIndex::search(const QString &string, const bool &isValid) const
{
    QStringList &&words = splitString(string, separators, case_sensitive);

    unordered_map<Index,Score> result_map;
    if (words.empty()){
        for (const auto &string_index_item : index.strings){
            auto score = (Score)(1.0/(double)string_index_item.max_match_len * RankItem::MAX_SCORE);
            if(const auto &[it, success] = result_map.emplace(string_index_item.item, score); !success)
                if (it->second < score)
                    it->second = score;
        }
    }
    else
    {
        struct StringMatch {
            StringMatch(Index i, Position p, uint16_t ml)
                    : index(i), position(p), match_len(ml){}
            Index index; Position position; uint16_t match_len;
        };

        auto invert = [](const vector<WordMatch> &word_matches){
            vector<StringMatch> string_matches;
            for (const auto &word_match : word_matches)
                for (const auto &occurrence : word_match.word_index_item.occurrences)
                    string_matches.emplace_back(occurrence.index, occurrence.position, word_match.match_length);
            sort(string_matches.begin(), string_matches.end(),
                 [](const auto &l, const auto &r){ return l.index < r.index; });
            return string_matches;
        };

        shared_lock lock(mutex);
        vector<StringMatch> left_matches = invert(getWordMatches(words[0], isValid));

        // In case of multiple words intersect. Todo: user chooses strategy
        for (int w = 1; w < words.size(); ++w) {

            if (!isValid || left_matches.empty())
                return {};

            vector<StringMatch> right_matches = invert(getWordMatches(words[w], isValid));

            if (right_matches.empty())
                return {};

            vector<StringMatch> intermediate_matches;
            for (auto lit = left_matches.cbegin(); lit != left_matches.cend();) {

                // Build a range of upcoming left_matches with same index
                auto elit = lit;
                while(elit != left_matches.cend() && lit->index==elit->index)
                    ++elit;

                // Get the range of equal string matches on the right side
                const auto &[eq_begin, eq_end] =
                        equal_range(right_matches.cbegin(), right_matches.cend(), *lit,
                                    [](const StringMatch &l, const StringMatch &r) { return l.index < r.index; });

                // If no match on the right side continue with next leftmatch
                if (eq_begin == eq_end){
                    lit = elit;
                    continue;
                }

                //
                for (;lit != elit; ++lit)
                    //for (const auto &right_matcht : std::ranges::subrange(eq_begin,eq_end))
                    for (auto rit = eq_begin; rit != eq_end; ++rit)
                        if (lit->position < rit->position)  // Sequence check
                            intermediate_matches.emplace_back(rit->index, rit->position,
                                                              rit->match_len + lit->match_len);

            }

            left_matches = ::move(intermediate_matches);
        }

        // Build the list of matched items with their highest scoring match
        for (const auto &match : left_matches) {
            auto score = (Score)((double)match.match_len / index.strings[match.index].max_match_len * RankItem::MAX_SCORE);
            if (const auto &[it, success] = result_map.emplace(index.strings[match.index].item, score);
                    !success && it->second < score) // update if exists
                it->second = score;
        }

    }

    // Convert results to return type
    vector<albert::RankItem> result;
    result.reserve(result_map.size());
    for (const auto &[item_idx, score] : result_map)
        result.emplace_back(index.items[item_idx], score);

    return result;
}
