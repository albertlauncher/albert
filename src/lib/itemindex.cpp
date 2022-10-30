// Copyright (c) 2021-2022 Manuel Schneider

#include "levenshtein.h"
#include "itemindex.h"
#include <QRegularExpression>
#include <map>
using namespace std;
using namespace albert;

static constexpr uint MAX_SCORE = std::numeric_limits<Score>::max();

static QStringList splitString(const QString &string, const QString &separators, bool case_sensitive = false)
{
    return ((case_sensitive) ? string.toLower(): string).split(QRegularExpression(separators), Qt::SkipEmptyParts);
}

static vector<QString> ngrams_for_word(const QString &word, uint n)
{
    vector<QString> ngrams;
    ngrams.reserve(word.size());
    auto padded = QString("%1%2").arg(QString(n - 1, ' '), word);
    for (int i = 0; i < word.size(); ++i){
        QString ngram{padded.mid(i, n)};
        ngram.shrink_to_fit();
        ngrams[i] = ngram;
    }
    return ngrams;
}

ItemIndex::ItemIndex(map<SharedItem,map<QString,Score>> &&index_items,
                     const QString &separators,
                     bool case_sensitive,
                     uint n,
                     uint error_tolerance_divisor)
    : case_sensitive(case_sensitive), error_tolerance_divisor(error_tolerance_divisor), separators(separators), n(n)
{
    // TODO C++20 allows emplace with aggregate initialization, check below (emplace(_back) push_back etc…)

    map<QString,WordIndexItem> word_index_;  // temporary associative word index

    Index item_idx = 0;
    Index string_idx = 0;
    for (const auto &[item, index_strings] : index_items) {

        item_index.emplace_back(item);  // populate item index

        for (const auto &[string, max_score]: index_strings) {
            QStringList &&words = splitString(string, separators, case_sensitive);

            // String index: add items and scores. Word indices not yet known, reserve for random access (1)
            string_index.emplace_back(
                    StringIndexItem{vector<Index>{static_cast<uint>(words.length()), 0}, item_idx, max_score});

            string_index.back().words.shrink_to_fit();

            // Word index: add occurrences in strings (word will be set later when uniq (2))
            Position pos = 0;
            for (const auto &word : words)
                word_index_[word].occurrences.push_back({string_idx, pos++});

            ++string_idx;
        }
        ++item_idx;
    }

    Index word_idx = 0;
    for (auto &[word, word_index_item] : word_index_) {

        // String index: Fill encoded words (1)
        for (const auto &word_occ: word_index_item.occurrences)
            string_index[word_occ.index].words[word_occ.position] = word_idx;

        // Word index: fill with lexical words and build the "real" index
        word_index_item.word = word;  // (2)
        word_index_item.word.shrink_to_fit();
        word_index_item.occurrences.shrink_to_fit();
        word_index.emplace_back(move(word_index_item));
        ++word_idx;
    }

    if (error_tolerance_divisor){
        // build q_gram_index
        for (word_idx = 0; word_idx < word_index.size(); ++word_idx) {
            vector<QString> ngrams(ngrams_for_word(word_index[word_idx].word, n));
            for (Position pos = 0 ; pos < ngrams.size(); ++pos)
                ngram_index[ngrams[pos]].push_back({word_idx, pos});
        }
    }

    // Squeeze memory
    item_index.shrink_to_fit();
    string_index.shrink_to_fit();
    word_index.shrink_to_fit();
    for (auto &[ngram, word_refs] : ngram_index)
        word_refs.shrink_to_fit();
}

std::vector<ItemIndex::StringMatch> ItemIndex::getWordMatches(const QString &word) const
{
    vector<StringMatch> matches;
    uint word_length = word.length();

    // Get range of perfect prefix match words
    const auto &[eq_begin, eq_end] = equal_range(word_index.cbegin(), word_index.cend(), WordIndexItem{word, {}},
                                                 [l=word_length](const WordIndexItem &a, const WordIndexItem &b) {
                                                     return QStringView{a.word}.left(l) < QStringView{b.word}.left(l);
                                                 });

    // Store perfect prefix match words
    for (auto it = eq_begin; it != eq_end; ++it)
        for (const Location &word_occ: it->occurrences)
            matches.push_back({word_occ.index, word_occ.position, word_length});

    // Get the (fuzzy) prefix matches
    if (error_tolerance_divisor) {
        Index prefix_match_first_id = eq_begin - word_index.begin();  // Ignore interval. closed begin [
        Index prefix_match_last_id = eq_end - word_index.begin();  // Ignore interval. open end )

        // Get the words referenced by each nGram and count the ngrams where position < word_length.
        vector<QString> ngrams(ngrams_for_word(word, n));
        unordered_map<Index,uint> word_match_count;

        for (const QString &n_gram: ngrams) {
            try {
                for (const auto &ngram_occ: ngram_index.at(n_gram)) {
                    // Exclude the existing perfect matches
                    if (ngram_occ.index < prefix_match_first_id || ngram_occ.index >= prefix_match_last_id)
                        continue;

                    if (ngram_occ.position < static_cast<Position>(word_length))
                        ++word_match_count[ngram_occ.index];
                    else
                        break;
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
        uint allowed_errors = static_cast<uint>(static_cast<float>(word_length) / error_tolerance_divisor);
        uint minimum_match_count = word_length - allowed_errors * n;
        Levenshtein levenshtein;
        for (const auto &[word_idx, ngram_count]: word_match_count) {
            if (ngram_count < minimum_match_count)
                continue;

            if (auto edit_distance = levenshtein.computePrefixEditDistanceWithLimit(word, word_index[word_idx].word,
                                                                                    allowed_errors);
                    edit_distance > allowed_errors)
                continue;
            else
                for (const auto &word_occ: word_index[word_idx].occurrences)
                    matches.push_back({word_occ.index, word_occ.position, word_length-edit_distance});
        }
    }
    return matches;
}

std::vector<Match> ItemIndex::search(const QString &string) const
{
    QStringList &&words = splitString(string, separators, case_sensitive);
    if (words.empty())
        return {};

    unordered_map<Index,vector<StringMatch>> w2s_matches;

    // Get initial string matches for intersection
    for (const auto &word_match : getWordMatches(words[0]))
        w2s_matches[word_match.index].emplace_back(word_match);

    // Intersect further sets of string matches with sequence checks
    for (int i = 1; i < words.size(); ++i) {
        unordered_map<Index,vector<StringMatch>> new_matches;
        for (const auto &r: getWordMatches(words[i])){
            try {
                for (const auto &l : w2s_matches.at(r.index))
                    if (l.position < r.position)  // Sequence check TODO configurable
                        new_matches[l.index].push_back({r.index, r.position, r.match_length + l.match_length});
            } catch (const out_of_range&) {
                // Previous words did not match this string
            }
        }
        if (new_matches.empty())
            return {};
        else
            w2s_matches = move(new_matches);
    }

    // Build the list of matched items with their highest scoring match
    map<Index,Score> item_scores;
    for (const auto &[string_idx, match_vector] : w2s_matches) {

        // Compute the amount of max matchable characters for the score (divisor)
        uint max_match_len = 0;
        for (auto word_idx : string_index[string_idx].words)
            max_match_len += word_index[word_idx].word.size();

        // Store the max score of all matches
        Score score = 0;
        for (const auto &match: match_vector)
            score = max(score, (Score)((double)match.match_length / max_match_len * string_index[string_idx].max_score));

        if (const auto &[it, emplace] = item_scores.emplace(string_index[string_idx].item, score); !emplace)
            it->second = score;
    }

    // Convert results in desired type
    vector<Match> result;
    result.reserve(item_scores.size());
    for (auto &[item_idx, score] : item_scores)
        result.emplace_back(Match{item_index[item_idx], score});
    return result;
}
