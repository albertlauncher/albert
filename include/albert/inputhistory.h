// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <QString>
#include <albert/export.h>
#include <memory>

namespace albert
{

///
/// Input history class.
///
/// Stores input strings and provides a search iterator.
///
class ALBERT_EXPORT InputHistory final : public QObject
{
    Q_OBJECT

public:

    InputHistory(const QString &path = {});
    ~InputHistory() override;

    ///
    /// Add text to history search.
    ///
    /// Skips empty strings and drops duplicates.
    ///
    /// @param str The string to add.
    ///
    Q_INVOKABLE void add(const QString& str);

    ///
    /// Get next history item matching the pattern.
    ///
    /// @param pattern A pattern used to filter the history items.
    /// @returns The next history item matching the pattern or empty string.
    ///
    Q_INVOKABLE QString next(const QString &pattern = QString{});

    ///
    /// Get previous history item matching the pattern.
    ///
    /// @param pattern A pattern used to filter the history items.
    /// @returns The previous history item matching the pattern or empty string.
    ///
    Q_INVOKABLE QString prev(const QString &pattern = QString{});

    ///
    /// Reset history search.
    ///
    Q_INVOKABLE void resetIterator();

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
