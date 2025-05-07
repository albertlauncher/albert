// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <QRegularExpression>

namespace albert::util
{

// Doxygen does not like raw strings
// QRegularExpression(R"([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)");
const QRegularExpression default_separator_regex("([\\s\\\\/\\-\\[\\](){}#!?<>\"'=+*.:,;_]+)");

///
/// Configuration for string matching.
///
/// Initialize with designated initializers to avoid hard to find bugs on future changes.
///
/// \sa \ref Matcher, \ref IndexQueryHandler
///
class ALBERT_EXPORT MatchConfig
{
public:

    ///
    /// Match strings error tolerant.
    ///
    bool fuzzy = false;

    ///
    /// Match strings case insensitive.
    ///
    bool ignore_case = true;

    ///
    /// Match strings independent of their order.
    ///
    bool ignore_word_order = true;

    ///
    /// Match strings normalized.
    ///
    bool ignore_diacritics = true;

    ///
    /// The separator regex used to split the compared strings.
    ///
    QRegularExpression separator_regex = default_separator_regex;

    ///
    /// The error tolerance.
    ///
    /// This hardcodes the error tolerance on enabled `fuzzy` to 25%.
    ///
    static const uint error_tolerance_divisor = 4;
};

} // namespace albert
