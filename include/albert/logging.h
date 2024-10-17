// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(AlbertLoggingCategory)

/// @brief Defines the logging category used in DEBG, INFO, WARN, CRIT, GWARN, GCRIT
/// @param name The name of the logging category
#define ALBERT_LOGGING_CATEGORY(name) Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert." name)

/// @brief Creates a log object (level debug) you can use to pipe text into (<<).
#define DEBG qCDebug(AlbertLoggingCategory,).noquote()

/// @brief Creates a log object (level info) you can use to pipe text into (<<).
#define INFO qCInfo(AlbertLoggingCategory,).noquote()

/// @brief Creates a log object (level warning) you can use to pipe text into (<<).
#define WARN qCWarning(AlbertLoggingCategory,).noquote()

/// @brief Creates a log object (level critial) you can use to pipe text into (<<).
#define CRIT qCCritical(AlbertLoggingCategory,).noquote()

#define cred     "\x1b[31m"
#define cgreen   "\x1b[32m"
#define cyellow  "\x1b[33m"
#define cblue    "\x1b[34m"
#define cmagenta "\x1b[35m"
#define ccyan    "\x1b[36m"
#define creset   "\x1b[0m"
