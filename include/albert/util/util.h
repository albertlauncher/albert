// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../export.h"
#include <QStringList>

namespace albert
{
ALBERT_EXPORT void openWebsite();

ALBERT_EXPORT void openUrl(const QString &url);

ALBERT_EXPORT void openIssueTracker();

ALBERT_EXPORT void setClipboardText(const QString &text);

ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline, const QString &working_dir = {});
}
