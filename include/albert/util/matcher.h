// SPDX-FileCopyrightText: 2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
class MatcherPrivate;

namespace albert
{
class Item;


class ALBERT_EXPORT Match final
{
public:
    using Score = double;

    Match(const Score score) : score_(score) {}

    inline operator bool() const { return isMatch(); }
    inline bool isMatch() const { return score_ >= 0.0; }
    inline bool isEmptyMatch() const { return qFuzzyCompare(score_, 0.0); }
    inline bool isExactMatch() const { return qFuzzyCompare(score_, 1.0); }
    inline Score score() const { return score_; }

private:

    const Score score_;
};


class ALBERT_EXPORT Matcher final
{
public:

    Matcher(const QString &query);
    ~Matcher();

    Match match(const Item &item) const;

    Match match(const QString &string) const;

private:

    ALBERT_NO_EXPORT std::unique_ptr<MatcherPrivate> d;
    friend class ALBERT_NO_EXPORT QueryEngine;

};

}
