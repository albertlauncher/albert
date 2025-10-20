// SPDX-FileCopyrightText: 2025 Manuel Schneider

#include <QString>
namespace albert { class MatchConfig; }

QStringList preprocessQuery(QString s, const albert::MatchConfig &config);

QStringList preprocessQueryLegacy(QString s);

