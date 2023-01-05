// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>

namespace albert
{
ALBERT_EXPORT void show(const QString &text = QString());

ALBERT_EXPORT void hide();

ALBERT_EXPORT void toggle();

ALBERT_EXPORT void showSettings();

ALBERT_EXPORT void restart();

ALBERT_EXPORT void quit();

ALBERT_EXPORT void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false);

ALBERT_EXPORT void sendTrayNotification(const QString &title, const QString &message, int msTimeoutHint);
}
