// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QRegularExpression>
#include <QString>
#include <albert/export.h>
#include <albert/matchconfig.h>
class MatcherPrivate;

namespace albert
{
class Item;

///
/// The Match class.
///
/// Utility class encapsulating and augmenting the match score.
///
/// Some nifty features:
/// - The bool type conversion evaluates to isMatch()
/// - The Score/double type conversion seamlessly uses the score
///
class ALBERT_EXPORT Match final
{
public:
    using Score = double;

    Match() : score_(-1.) {}
    Match(const Score score) : score_(score) {}
    Match(const Match &o) = default;
    Match(Match &&o) = default;
    Match &operator=(const Match &o) = default;
    Match &operator=(Match &&o) = default;

    inline operator bool() const { return isMatch(); }
    inline explicit operator Score() const { return score_; }
    inline bool isMatch() const { return score_ >= 0.0; }
    inline bool isEmptyMatch() const { return qFuzzyCompare(score_, 0.0); }
    inline bool isExactMatch() const { return qFuzzyCompare(score_, 1.0); }
    inline Score score() const { return score_; }

private:

    Score score_;
};


///
/// The Matcher class.
///
/// Use this class to get unified user experience in match behavior.
///
/// @note Experimental WIP
///
class ALBERT_EXPORT Matcher final
{
public:

    Matcher(const QString &query, MatchConfig config = {});
    Matcher(Matcher &&o);
    Matcher &operator=(Matcher &&o);
    ~Matcher();

    Match match(const Item &item) const;
    Match match(const QString &string) const;

private:

    std::unique_ptr<MatcherPrivate> d;

};

}
