// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QLoggingCategory>

#if defined(PROJECT_NAME)
#define ALBERT_LOGGING_CATEGORY PROJECT_NAME
#else
#define ALBERT_LOGGING_CATEGORY "albert"
#endif

Q_DECLARE_LOGGING_CATEGORY(LoggingCategory)

///
#define ALBERT_LOGGING Q_LOGGING_CATEGORY(LoggingCategory, ALBERT_LOGGING_CATEGORY)

/// @brief Creates a log object (level debug) you can use to pipe text into (<<).
#define DEBG qCDebug(LoggingCategory,).noquote()

/// @brief Creates a log object (level info) you can use to pipe text into (<<).
#define INFO qCInfo(LoggingCategory,).noquote()

/// @brief Creates a log object (level warning) you can use to pipe text into (<<).
#define WARN qCWarning(LoggingCategory,).noquote()

/// @brief Creates a log object (level critial) you can use to pipe text into (<<).
#define CRIT qCCritical(LoggingCategory,).noquote()

/// @brief Logs and shows a messagebox (level warning).
/// @param message The message
#define GWARN(message) { WARN << message; QMessageBox::warning(nullptr, qApp->applicationDisplayName(), message); }

/// @brief Logs and shows a messagebox (level critical).
/// @param message The message
#define GCRIT(message) { CRIT << message; QMessageBox::critical(nullptr, qApp->applicationDisplayName(), message); }
