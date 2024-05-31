// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
class MatcherPrivate;

namespace albert
{
class Item;


class ALBERT_EXPORT Match final
{
public:
    using Score = double;

    Match(const Score score) : score_(score) {}
    Match() = default;
    Match(const Match &o) = default;
    Match(Match &&o) = default;
    Match &operator=(const Match &o) = default;
    Match &operator=(Match &&o) = default;

    inline operator bool() const { return isMatch(); }
    inline bool isMatch() const { return score_ >= 0.0; }
    inline bool isEmptyMatch() const { return qFuzzyCompare(score_, 0.0); }
    inline bool isExactMatch() const { return qFuzzyCompare(score_, 1.0); }
    inline Score score() const { return score_; }

private:

    Score score_;
};


class ALBERT_EXPORT Matcher final
{
public:

    Matcher(const QString &query);
    ~Matcher();

    Match match(const Item &item) const;

    Match match(const QString &string) const;

private:

    std::unique_ptr<MatcherPrivate> d;
    friend class ALBERT_NO_EXPORT QueryEngine;

};

}
