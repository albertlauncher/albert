// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>
#include <QStringList>
#include <map>
#include <memory>

namespace albert
{
class ExtensionRegistry;
ALBERT_EXPORT ExtensionRegistry &extensionRegistry();

ALBERT_EXPORT void show(const QString &text = QString());

ALBERT_EXPORT void hide();

ALBERT_EXPORT void toggle();

ALBERT_EXPORT void showSettings();

ALBERT_EXPORT void restart();

ALBERT_EXPORT void quit();

ALBERT_EXPORT void openWebsite();

ALBERT_EXPORT void openUrl(const QString &url);

ALBERT_EXPORT void openIssueTracker();

ALBERT_EXPORT void setClipboardText(const QString &text);

ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline, const QString &working_dir = {});

ALBERT_EXPORT void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false);

//ALBERT_EXPORT void sendTrayNotification(const QString &title, const QString &message, const QIcon &icon) override;
}
