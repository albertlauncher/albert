// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QString>

namespace platform
{

void initPlatform();

void initNativeWindow(unsigned long long wid);

/// Runs an AppleScript and returns the result. Throws runtime_error on failure.
QString runAppleScript(const QString &script);

}
