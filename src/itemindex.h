// Copyright (c) 2021-2023 Manuel Schneider

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
class ItemIndex : public Index
{
public:
    explicit ItemIndex(QString separators, bool case_sensitive, uint n, uint error_tolerance_divisor);

    void setItems(std::vector<albert::IndexItem> &&) override;
    std::vector<albert::RankItem> search(const QString &string, const bool &isValid) const override;

private:
    using Index = uint32_t;
    using Position = uint16_t;
    struct Location {
        Location(Index i, Position p) : index(i), position(p) {}
        Index index;
        Position position;
    };

    struct StringIndexItem {  // inverted item index, s_idx > ([w_idx], [(i_idx, s_scr)])
        StringIndexItem(Index i, uint16_t mml)
            : item(i), max_match_len(mml) {}
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
        WordMatch(const WordIndexItem &wii, uint ml)
            : word_index_item(wii), match_length(ml){}
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
