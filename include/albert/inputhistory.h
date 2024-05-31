// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <QStringList>
#include <albert/export.h>

namespace albert
{

/// Input history class.
/// Stores input stings and provides a search iterator.
class ALBERT_EXPORT InputHistory final : public QObject  /// Input line history for frontends.
{
    Q_OBJECT
public:
    explicit InputHistory();
    ~InputHistory() override;

    /// Add text to history search.
    /// @note Skips empty strings and drops duplicates.
    Q_INVOKABLE void add(const QString& str);

    /// Next distinct history item.
    /// @note Skips perfect matches.
    Q_INVOKABLE QString next(const QString &substring = QString{});

    /// Previous distinct history item.
    /// @note Skips perfect matches.
    Q_INVOKABLE QString prev(const QString &substring = QString{});

    /// Reset history search.
    Q_INVOKABLE void resetIterator();

private:
    QString file_path;
    QStringList lines_;
    int currentLine_;
};

}
