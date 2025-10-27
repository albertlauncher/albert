// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(AlbertLoggingCategory)

///
/// Defines the logging category with the name _name_.
///
#define ALBERT_LOGGING_CATEGORY(name) Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert." name)

///
/// Creates a log object (level debug) you can use to pipe text into (<<).
///
#define DEBG qCDebug(AlbertLoggingCategory,).noquote()

///
/// Creates a log object (level info) you can use to pipe text into (<<).
///
#define INFO qCInfo(AlbertLoggingCategory,).noquote()

///
/// Creates a log object (level warning) you can use to pipe text into (<<).
///
#define WARN qCWarning(AlbertLoggingCategory,).noquote()

///
/// Creates a log object (level critial) you can use to pipe text into (<<).
///
#define CRIT qCCritical(AlbertLoggingCategory,).noquote()
