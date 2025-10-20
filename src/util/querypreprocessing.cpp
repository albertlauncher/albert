// SPDX-FileCopyrightText: 2025 Manuel Schneider

#include "matchconfig.h"
#include "querypreprocessing.h"
#include <QRegularExpression>
#include <QTextBoundaryFinder>
using namespace albert;

QStringList preprocessQuery(QString s, const MatchConfig &config)
{
    if (config.ignore_diacritics)
    {
        static const QRegularExpression re(
            "(["
            R"(\x{0300}-\x{036f})"  // diacritical marks
            "\u00AD"  // Soft hyphen
            "])");
        s = s.normalized(QString::NormalizationForm_D).remove(re);
    }
    else
    {
        // Remove soft hyphens
        s.remove(QChar(0x00AD));
    }

    if (config.ignore_case)
        s = s.toLower();

    QTextBoundaryFinder finder(QTextBoundaryFinder::Word, s);
    QStringList tokens;
    qsizetype begin = 0;
    for (qsizetype end = finder.toNextBoundary();
         end != -1;
         end = finder.toNextBoundary())
    {
        if (begin != end)
        {
            // Only add non-space tokens
            // Assumes that the tokes that either are all spaces or non-spaces
            if(!s[begin].isSpace())
                tokens << s.mid(begin, end - begin);
            begin = end;
        }
    }

    if (config.ignore_word_order)
        tokens.sort();

    return tokens;
}

QStringList preprocessLegacy(QString s)
{
    static const QRegularExpression legacy_regex =
        QRegularExpression(QStringLiteral(R"(([\s\\/\-\[\](){}#!?<>"'=+*.:,;_]+))"));
    s.remove(QChar(0x00AD));  // Soft hyphen
    return s.split(legacy_regex, Qt::SkipEmptyParts);
}
