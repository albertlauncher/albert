// Copyright (c) 2024-2024 Manuel Schneider

#include "desktopentryparser.h"
#include <QFile>
#include <albert/logging.h>
using namespace Qt::StringLiterals;
using namespace albert::detail;
using namespace std;

DesktopEntryParser::DesktopEntryParser(const QString &path)
{
    if (QFile file(path); file.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QTextStream stream(&file);
        QString currentGroup;
        for (QString line=stream.readLine(); !line.isNull(); line=stream.readLine())
        {
            line = line.trimmed();

            if (line.startsWith(u'#') || line.isEmpty())
                continue;

            if (line.startsWith(u"["))
            {
                currentGroup = line.mid(1,line.size()-2).trimmed();
                continue;
            }

            data[currentGroup].emplace(line.section(u'=', 0,0).trimmed(),
                                       line.section(u'=', 1, -1).trimmed());
        }
        file.close();
    }
    else
        throw runtime_error(u"Failed opening file '%1': %2"_s
                                .arg(path, file.errorString()).toStdString());
}

QString DesktopEntryParser::getRawValue(const QString &section, const QString &key) const
{
    class SectionDoesNotExist : public std::out_of_range { using out_of_range::out_of_range; };
    class KeyDoesNotExist : public std::out_of_range { using out_of_range::out_of_range; };

    try {
        auto &s = data.at(section);
        try {
            return s.at(key);
        } catch (const out_of_range&) {
            throw KeyDoesNotExist(u"Section '%1' does not contain a key '%2'."_s
                                      .arg(section, key).toStdString());
        }
    } catch (const out_of_range&) {
        throw SectionDoesNotExist(u"Desktop entry does not contain a section '%1'."_s
                                      .arg(section).toStdString());
    }
}

QString DesktopEntryParser::getEscapedValue(const QString &section, const QString &key) const
{
    QString result;

    auto unescaped = getRawValue(section, key);
    for (auto it = unescaped.cbegin(); it != unescaped.cend();)
    {
        if (*it == u'\\'){
            ++it;
            if (it == unescaped.cend())
                break;
            else if (*it==u's')
                result.append(u' ');
            else if (*it==u'n')
                result.append(u'\n');
            else if (*it==u't')
                result.append(u'\t');
            else if (*it==u'r')
                result.append(u'\r');
            else if (*it==u'\\')
                result.append(u'\\');
        }
        else
            result.append(*it);
        ++it;
    }

    return result;
}

QString DesktopEntryParser::getString(const QString &section, const QString &key) const
{
    return getEscapedValue(section, key);
}

QString DesktopEntryParser::getLocaleString(const QString &section, const QString &key)
{
    // https://wiki.ubuntu.com/UbuntuDevelopment/Internationalisation/Packaging#Desktop_Entries


    // TODO: Properly fetch the localestring
    //       (lang_COUNTRY@MODIFIER, lang_COUNTRY, lang@MODIFIER, lang, default value)

    try {
        return getEscapedValue(section, u"%1[%2]"_s.arg(key, locale.name()));
    } catch (const out_of_range&) { }

    try {
        return getEscapedValue(section, u"%1[%2]"_s.arg(key, locale.name().left(2)));
    } catch (const out_of_range&) { }

    QString unlocalized = getEscapedValue(section, key);

    try {
        auto domain = getEscapedValue(section, u"X-Ubuntu-Gettext-Domain"_s);
        // The resulting string is statically allocated and must not be modified or freed
        // Returns msgid on lookup failure
        // https://linux.die.net/man/3/dgettext
        return QString::fromUtf8(dgettext(domain.toStdString().c_str(),
                                          unlocalized.toStdString().c_str()));
    } catch (const out_of_range&) { }

    return unlocalized;
}

QString DesktopEntryParser::getIconString(const QString &section, const QString &key)
{
    return getEscapedValue(section, key);
}

bool DesktopEntryParser::getBoolean(const QString &section, const QString &key)
{
    auto raw = getRawValue(section, key);  // throws
    if (raw == u"true"_s)
        return true;
    else if (raw == u"false"_s)
        return false;
    else
        throw runtime_error(u"Value for key '%1' in section '%2' is neither true nor false."_s
                                .arg(key, section).toStdString());
}

double DesktopEntryParser::getNumeric(const QString &, const QString &)
{
    throw runtime_error("Not implemented.");
}

optional<QStringList> DesktopEntryParser::splitExec(const QString &s) noexcept
{
    QStringList tokens;
    QString token;
    auto c = s.begin();

    while (c != s.end())
    {
        if (*c == QChar::Space)  // separator
        {
            if (!token.isEmpty())
            {
                tokens << token;
                token.clear();
            }
        }

        else if (*c == u'"')  // quote
        {
            ++c;

            while (c != s.end())
            {
                if (*c == u'"')  // quote termination
                    break;

                else if (*c == u'\\')  // escape
                {
                    ++c;
                    if(c == s.end())
                    {
                        WARN << u"Unterminated escape in %1"_s.arg(s);
                        return {};  // unterminated escape
                    }

                    else if (uR"("`$\)"_s.contains(*c))
                        token.append(*c);

                    else
                    {
                        WARN << u"Invalid escape '%1' at '%2': %3"_s
                                    .arg(*c).arg(distance(c, s.begin())).arg(s);
                        return {};  // invalid escape
                    }
                }

                else
                    token.append(*c);  // regular char

                ++c;
            }

            if (c == s.end())
            {
                WARN << u"Unterminated escape in %1"_s.arg(s);
                return {};  // unterminated quote
            }
        }

        else
            token.append(*c);  // regular char

        ++c;

    }

    if (!token.isEmpty())
        tokens << token;

    return tokens;
}
