// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <QString>
#include <map>
#include <unordered_map>
namespace albert { class Item; }
using Score = uint16_t;

class ItemIndex
{
public:
    explicit ItemIndex(std::map<std::shared_ptr<albert::Item>, std::map<QString,uint16_t>> &&index_items,
                       const QString &separators, bool case_sensitive,
                       uint n, uint error_tolerance_divisor);  // ceil(len(word)/error_tolerance_divisor))
    ItemIndex() = default;
    ItemIndex(const ItemIndex &) = default;
    ItemIndex(ItemIndex &&) = default;
    ItemIndex &operator=(const ItemIndex &) = default;
    ItemIndex &operator=(ItemIndex &&) = default;

    std::vector<std::pair<std::shared_ptr<albert::Item>, Score>> search(const QString &string) const;

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

    std::vector<std::shared_ptr<albert::Item>> item_index;
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
