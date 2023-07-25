// Copyright (c) 2023 Manuel Schneider
#pragma once
#include <QString>
namespace albert { class Frontend; }

namespace platform
{
void initPlatform();

void initNativeWindow(unsigned long long wid);

void sendNotification(const QString &title, const QString &message, int msTimeoutHint);

}
