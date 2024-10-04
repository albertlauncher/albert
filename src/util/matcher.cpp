// SPDX-FileCopyrightText: 2024 Manuel Schneider

#include "item.h"
#include "levenshtein.h"
#include "matchconfig.h"
#include "matcher.h"
#include <QRegularExpression>
#include <QStringList>
using namespace albert;
using namespace std;

class MatcherPrivate
{
public:

    MatchConfig config;
    const QString string;
    mutable Levenshtein levenshtein;
    QStringList tokens;

    QStringList tokenize(QString s) const
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

    void updateTokens() { tokens = tokenize(string); }

    Match match(const QString &s) const
    {
        // Empty query is a 0 score (epsilon) match
        if (string.isEmpty())
            return {0.};

        // Do not match strings containing only separators
        if (tokens.isEmpty())
            return {-1.};

        QStringList other_tokens = tokenize(s);

        double matched_chars = 0;
        double total_chars = 0;

        auto it = tokens.begin();
        auto oit = other_tokens.begin();

        while (it != tokens.end() && oit != other_tokens.end())
        {

            // if the query word is longer it cant be a prefix
            if ((it->size() <= oit->size()))
            {
                // check if the query word is a prefix of the matched word
                if(config.fuzzy)
                {
                    uint allowed_errors = it->size() / config.error_tolerance_divisor;
                    auto edit_distance = levenshtein.computePrefixEditDistanceWithLimit(
                                *it, *oit, allowed_errors);
                    if (edit_distance <= allowed_errors)
                        // Accumulate matched chars and move to the next matcher word
                        matched_chars += it++->size() - edit_distance;
                }
                else  // non fuzzy
                {
                    if (oit->startsWith(*it))
                        // Accumulate matched chars and move to the next matcher word
                        matched_chars += it++->size();
                }
            }

            total_chars += oit->size();
            ++oit;  // move to the next matched word
        }

        // Count chars of the left other_tokens (if any)
        while (oit != other_tokens.end())
            total_chars += oit++->size();

        // if all matcher words have been consumed this is a match
        if (it == tokens.end())
            return {matched_chars/total_chars};

        return {-1.};
    }
};

Matcher::Matcher(const QString &query, MatchConfig config):
    d(new MatcherPrivate{
      .config = ::move(config),
      .string = query,
      .levenshtein = {},
      .tokens = {}
    })
{ d->updateTokens(); }

Matcher::Matcher(Matcher &&o) = default;

Matcher::~Matcher() = default;

Matcher &Matcher::operator=(Matcher &&o) = default;

Match Matcher::match(const Item &item) const { return d->match(item.text()); }

Match Matcher::match(const QString &s) const { return d->match(s); }
