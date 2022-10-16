// Copyright (C) 2022 Manuel Schneider

#pragma once
#include <QLoggingCategory>

#if defined(PROJECT_NAME)
#define ALBERT_QLC_ID PROJECT_NAME
#else
#define ALBERT_QLC_ID "albert"
#endif

#define ALBERT_DECLARE_LOGGING_CATEGORY Q_DECLARE_LOGGING_CATEGORY(LogCat)
#define ALBERT_DEFINE_LOGGING_CATEGORY Q_LOGGING_CATEGORY(LogCat, ALBERT_QLC_ID)
ALBERT_DECLARE_LOGGING_CATEGORY

#define DEBG qCDebug(LogCat,).noquote()
#define INFO qCInfo(LogCat,).noquote()
#define WARN qCWarning(LogCat,).noquote()
#define CRIT qCCritical(LogCat,).noquote()

