// SPDX-FileCopyrightText: 2025-2026 Manuel Schneider

#include <QStringList>
#include "matchconfig.h"

QStringList preprocessQuery(const QString &string, const albert::MatchConfig &config = {});

QStringList preprocessQueryUntil2026(QString, const albert::MatchConfig &config = {});

QStringList preprocessQueryLegacy(QString string);

