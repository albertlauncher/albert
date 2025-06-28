// Copyright (c) 2024-2024 Manuel Schneider

#pragma once
#include <QLocale>
#include <QString>
#include <albert/export.h>
#include <map>
#include <optional>

namespace albert::detail {

/// Desktop entry parser
/// http://standards.freedesktop.org/desktop-entry-spec/latest/
class ALBERT_EXPORT DesktopEntryParser
{
public:

    DesktopEntryParser(const QString &path);

    /// Get and escape string according to spec
    ///
    /// Values of type string may contain all ASCII characters except for
    /// control characters.
    ///
    /// @returns The escaped string of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    QString getString(const QString &section, const QString &key) const;

    /// Get localestring according to spec
    ///
    /// Values of type localestring are user displayable, and are encoded in UTF-8.
    ///
    /// @returns The localestring of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    QString getLocaleString(const QString &section, const QString &key);

    /// Get iconstring according to spec
    ///
    /// Values of type iconstring are the names of icons; these may be
    /// absolute paths, or symbolic names for icons located using the
    /// algorithm described in the Icon Theme Specification. Such values
    /// are not user-displayable, and are encoded in UTF-8.
    ///
    /// @returns The iconstring of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    QString getIconString(const QString &section, const QString &key);

    /// Get boolean according to spec
    ///
    /// Values of type boolean must either be the string true or false.
    ///
    /// @returns The boolean of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    bool getBoolean(const QString &section, const QString &key);

    /// Get numeric according to spec
    ///
    /// Values of type numeric must be a valid floating point number as
    /// recognized by the %f specifier for scanf in the C locale.
    ///
    /// @returns The numeric of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    double getNumeric(const QString &, const QString &);

    /// Split an Exec string according to spec
    static std::optional<QStringList> splitExec(const QString &s) noexcept;

private:

    /// Get a raw, unescaped value for a section and key.
    ///
    /// @returns The raw, unescaped string of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    QString getRawValue(const QString &section, const QString &key) const;

    /// Get an escaped value for a section and key.
    ///
    /// The escape sequences \s, \n, \t, \r, and \\ are supported for values of
    /// type string, localestring and iconstring, meaning ASCII space, newline,
    /// tab, carriage return, and backslash, respectively.
    ///
    /// @returns The escaped string of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    QString getEscapedValue(const QString &section, const QString &key) const;

    std::map<QString, std::map<QString,QString>> data;
    QLocale locale;

};

}
