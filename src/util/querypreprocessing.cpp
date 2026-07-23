// SPDX-FileCopyrightText: 2025-2026 Manuel Schneider

#include "querypreprocessing.h"
#include <QRegularExpression>
#include <QTextBoundaryFinder>
using namespace albert;
using namespace std;

QStringList preprocessQuery(const QString &string, const MatchConfig &config)
{
    QString s = config.ignore_diacritics ? string.normalized(QString::NormalizationForm_D) : string;

    auto it = s.begin();

    for (const auto &c : as_const(s))

        // Format control characters (Cf)
        // https://www.unicode.org/reports/tr44/#GC_Values_Table
        if (const auto cat = c.category();
            cat == QChar::Other_Format)
            continue;

        // Diacritical marks
        else if (config.ignore_diacritics && cat == QChar::Mark_NonSpacing)
            continue;

        // Ignore undersores
        else if (static auto underscore = QChar(u'_');
            config.ignore_underscore && c == underscore)
            *(it++) = QChar::Space;

        else
            *(it++) = c;

    s.resize(distance(s.begin(), it));

    QStringList tokens;
    array<unsigned char, 512> buf;
    QTextBoundaryFinder finder(QTextBoundaryFinder::Word, s, buf.data(), buf.size());

    for (auto begin = 0ll, end = finder.toNextBoundary(); end != -1;
         begin = end, end = finder.toNextBoundary())
        // Only add non-space tokens, assumes either all spaces or not
        if(!s[begin].isSpace())
        {
            if (config.ignore_case)
                tokens << s.mid(begin, end - begin).toLower();
            else
                tokens << s.mid(begin, end - begin);
        }

    if (config.ignore_word_order)
        tokens.sort();

    return tokens;
}

QStringList preprocessQueryUntil2026(QString s, const MatchConfig &config)
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

QStringList preprocessQueryLegacy(QString s)
{
    static const QRegularExpression legacy_regex =
        QRegularExpression(QStringLiteral(R"(([\s\\/\-\[\](){}#!?<>"'=+*.:,;_]+))"));
    s.remove(QChar(0x00AD));  // Soft hyphen
    return s.split(legacy_regex, Qt::SkipEmptyParts);
}
