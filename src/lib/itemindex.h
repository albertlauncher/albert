// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "item.h"
#include "extension.h"
#include <QString>
#include <unordered_map>
#include <map>

using Score = uint8_t;

struct Match
{
    albert::SharedItem item;
    Score score;
};

class ItemIndex
{
public:
    explicit ItemIndex(std::map<albert::SharedItem,std::map<QString,Score>> &&index_items,
                       const QString &separators, bool case_sensitive,
                       uint n, uint error_tolerance_divisor);  // ceil(len(word)/error_tolerance_divisor))
    ItemIndex() = delete;
    ItemIndex(const ItemIndex &) = delete;
    ItemIndex(ItemIndex &&) = default;
    ItemIndex &operator=(const ItemIndex &) = delete;
    ItemIndex &operator=(ItemIndex &&) = default;

    std::vector<Match> search(const QString &string) const;

private:
    using Index = uint32_t;
    using Position = uint16_t;
    struct Location { Index index; Position position; };

    struct StringIndexItem {  // inverted item index, s_idx > ([w_idx], [(i_idx, s_scr)])
        std::vector<Index> words;
        Index item;
        Score max_score;
    };

    struct WordIndexItem {  // inverted string index, w_idx > (word, [(str_idx, w_pos)])
        QString word;
        std::vector<Location> occurrences;
    };

    std::vector<albert::SharedItem> item_index;
    std::vector<StringIndexItem> string_index;
    std::vector<WordIndexItem> word_index;
    std::unordered_map<QString,std::vector<Location>> ngram_index;

    struct StringMatch {
        Index index;
        Position position;
        uint match_length;
    };
    std::vector<StringMatch> getWordMatches(const QString &word) const;


    bool case_sensitive;
    uint error_tolerance_divisor;
    QString separators;
    uint n;
};
