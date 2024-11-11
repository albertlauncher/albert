// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <QRegularExpression>

namespace albert
{

///
/// Configuration for string matching.
///
/// \sa \ref Matcher, \ref IndexQueryHandler
///
class ALBERT_EXPORT MatchConfig
{
public:
    ///
    /// The separator regex used to split the compared strings.
    ///
    QRegularExpression separator_regex =
        //    QRegularExpression(R"([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)");
        // make doxygen happy
        QRegularExpression("([\\s\\\\/\\-\\[\\](){}#!?<>\"'=+*.:,;_]+)");

    ///
    /// Match strings case insensitive.
    ///
    bool ignore_case = true;

    ///
    /// Match strings normalized.
    ///
    bool ignore_diacritics = true;

    ///
    /// Match strings independent of their order.
    ///
    bool ignore_word_order = true;

    ///
    /// Match strings error tolerant.
    ///
    bool fuzzy = false;

    ///
    /// The error tolerance.
    ///
    /// This hardcodes the error tolerance on enabled `fuzzy` to 25%.
    ///
    static const uint error_tolerance_divisor = 4;
};

}
