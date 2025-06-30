// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QString>

namespace platform
{
void initPlatform();
void initNativeWindow(unsigned long long wid);
QString runAppleScript(const QString &script);
}
