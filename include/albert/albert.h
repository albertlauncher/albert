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

class Query;
ALBERT_EXPORT std::unique_ptr<Query> query(const QString &query);

ALBERT_EXPORT void show(const QString &text = QString());

ALBERT_EXPORT void showSettings();

ALBERT_EXPORT void restart();

ALBERT_EXPORT void quit();

ALBERT_EXPORT void openWebsite();

ALBERT_EXPORT void openIssueTracker();

ALBERT_EXPORT void setClipboardText(const QString &text);

ALBERT_EXPORT int runDetachedProcess(const QStringList &commandline,
                                     const QString &working_dir = {});

ALBERT_EXPORT void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false);

//ALBERT_EXPORT void sendTrayNotification(const QString &title, const QString &message, const QIcon &icon) override;

//ALBERT_EXPORT albert::Terminal &terminal() override;

//ALBERT_EXPORT albert::ExtensionRegistry &extensionRegistry() override;
}
