// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "index.h"
#include <QString>
#include <shared_mutex>
#include <unordered_map>
namespace albert {
class Item;
class RankItem;
class IndexItem;
}

// Thread safe index class
class ItemIndex : public albert::Index
{
public:
    explicit ItemIndex(QString separators, bool case_sensitive, uint n, uint error_tolerance_divisor);

    void setItems(std::vector<albert::IndexItem> &&) override;
    std::vector<albert::RankItem> search(const QString &string, const bool &isValid) const override;

private:
    using Index = uint32_t;
    using Position = uint16_t;
    struct Location {
        Location(Index index, Position position) : index(index), position(position) {}
        Index index;
        Position position;
    };

    struct StringIndexItem {  // inverted item index, s_idx > ([w_idx], [(i_idx, s_scr)])
        StringIndexItem(Index item, uint16_t max_match_len)
            : item(item), max_match_len(max_match_len) {}
        Index item;
        uint16_t max_match_len;
        //Score relevance;
    };

    struct WordIndexItem {  // inverted string index, w_idx > (word, [(str_idx, w_pos)])
        QString word;
        std::vector<Location> occurrences;
    };

    struct IndexData {
        std::vector<std::shared_ptr<albert::Item>> items;
        std::vector<StringIndexItem> strings;
        std::vector<WordIndexItem> words;
        std::unordered_map<QString, std::vector<Location>> ngrams;
    };

    struct WordMatch {
        WordMatch(const WordIndexItem &word_index_item, uint match_length)
            : word_index_item(word_index_item), match_length(match_length){}
        const WordIndexItem &word_index_item;
        uint16_t match_length;
    };

    mutable std::shared_mutex mutex;
    IndexData index;
    const bool case_sensitive;
    const uint error_tolerance_divisor;
    const QString separators;
    const uint n;

//    IndexData buildIndex(std::vector<albert::IndexItem> &&index_items) const;
    std::vector<WordMatch> getWordMatches(const QString &word, const bool &isValid) const;
};
