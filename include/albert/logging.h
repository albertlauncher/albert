// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(AlbertLoggingCategory)

/// @brief Defines the logging category used in DEBG, INFO, WARN, CRIT, GWARN, GCRIT
/// @param name The name of the logging category
#define ALBERT_LOGGING_CATEGORY(name) Q_LOGGING_CATEGORY(AlbertLoggingCategory, name, QtMsgType::QtDebugMsg)

/// @brief Creates a log object (level debug) you can use to pipe text into (<<).
#define DEBG qCDebug(AlbertLoggingCategory,).noquote()

/// @brief Creates a log object (level info) you can use to pipe text into (<<).
#define INFO qCInfo(AlbertLoggingCategory,).noquote()

/// @brief Creates a log object (level warning) you can use to pipe text into (<<).
#define WARN qCWarning(AlbertLoggingCategory,).noquote()

/// @brief Creates a log object (level critial) you can use to pipe text into (<<).
#define CRIT qCCritical(AlbertLoggingCategory,).noquote()

/// @brief Logs and shows a messagebox (level warning).
/// @param message The message
#define GWARN(message) do{ \
    WARN << message; \
    QMessageBox::warning(nullptr, qApp->applicationDisplayName(), message); \
} while(0)

/// @brief Logs and shows a messagebox (level critical).
/// @param message The message
#define GCRIT(message) do{ \
    CRIT << message; \
    QMessageBox::critical(nullptr, qApp->applicationDisplayName(), message); \
} while(0)
