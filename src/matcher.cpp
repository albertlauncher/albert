// SPDX-FileCopyrightText: 2024 Manuel Schneider

#include "albert/util/matcher.h"
#include "albert/query/item.h"
#include <QStringList>
using namespace std;
using namespace albert;



class MatcherPrivate
{
public:

    QStringList words;

    bool case_sensitive = global_config_case_sensitive;
    bool fuzzy = global_config_fuzzy;
    bool ignore_order = global_config_ignore_order;
    bool left_anchored = global_config_left_anchored;

    static bool global_config_case_sensitive;
    static bool global_config_fuzzy;
    static bool global_config_ignore_order;
    static bool global_config_left_anchored;
};

bool MatcherPrivate::global_config_case_sensitive = false;
bool MatcherPrivate::global_config_fuzzy = false;
bool MatcherPrivate::global_config_ignore_order = true;
bool MatcherPrivate::global_config_left_anchored = true;


Matcher::Matcher(const QString &query):
    d(make_unique<MatcherPrivate>())
{
    if (d->case_sensitive)
        d->words = query.split(" ", Qt::SkipEmptyParts);
    else
        d->words = query.toLower().split(" ", Qt::SkipEmptyParts);

    if (d->ignore_order)
        d->words.sort();
}

Matcher::~Matcher() = default;

Match Matcher::match(const Item &item) const
{
    return match(item.text());
}

Match Matcher::match(const QString &string) const
{
    // Empty query is a 0 score (epsilon) match
    if (d->words.isEmpty())
        return {0.};

    QStringList words;

    if (d->case_sensitive)
        words = string.split(" ", Qt::SkipEmptyParts);
    else
        words = string.toLower().split(" ", Qt::SkipEmptyParts);


    if (d->ignore_order)
        words.sort();

    auto sit = d->words.begin();
    auto oit = words.begin();
    double matched_chars = 0;
    double total_chars = 0;

    while (sit != d->words.end() && oit != words.end())
    {
        total_chars += oit->size();

        // if the query word is longer it cant be a prefix
        // check if the query word is a prefix of the matched word
        if ((sit->size() <= oit->size()) && oit->startsWith(*sit))
        {
            matched_chars += sit->size();
            ++sit;  // move to the next matcher word
        }

        ++oit;  // move to the next matched word
    }

    // if all matcher words have been consumed this is a match
    if (sit == d->words.end())
        return {matched_chars/total_chars};

    return {-1.};
}
