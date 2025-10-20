// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>

namespace albert::util
{

///
/// Configuration for string matching.
///
/// Initialize with designated initializers to avoid hard to find bugs on future API changes.
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

};

} // namespace albert
