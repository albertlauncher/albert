// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QRegularExpression>

namespace albert
{

struct MatchConfig
{
    QRegularExpression separator_regex =
            QRegularExpression(R"([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)");
    bool ignore_case = true;
    bool ignore_diacritics = true;
    bool ignore_word_order = true;
    uint error_tolerance_divisor = 0;
};

}
