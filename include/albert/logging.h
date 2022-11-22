// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QLoggingCategory>

#if defined(PROJECT_NAME)
#define ALBERT_LOGGING_CATEGORY PROJECT_NAME
#else
#define ALBERT_LOGGING_CATEGORY "albert"
#endif


Q_DECLARE_LOGGING_CATEGORY(LoggingCategory)
#define ALBERT_LOGGING Q_LOGGING_CATEGORY(LoggingCategory, ALBERT_LOGGING_CATEGORY)


#define DEBG qCDebug(LoggingCategory,).noquote()
#define INFO qCInfo(LoggingCategory,).noquote()
#define WARN qCWarning(LoggingCategory,).noquote()
#define CRIT qCCritical(LoggingCategory,).noquote()
