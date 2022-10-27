// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extensionregistry.h"
#include <QString>
#include <map>

namespace albert
{

class ExtensionRegistry;
ALBERT_EXPORT ExtensionRegistry &extensionRegistry();

class Query;
ALBERT_EXPORT Query* query(const QString &query);

ALBERT_EXPORT void show(const QString &text = QString());

ALBERT_EXPORT void showSettings();

ALBERT_EXPORT void restart();

ALBERT_EXPORT void quit();

//ALBERT_EXPORT void sendTrayNotification(const QString &title, const QString &message, const QIcon &icon) override;

//ALBERT_EXPORT albert::Terminal &terminal() override;

//ALBERT_EXPORT albert::ExtensionRegistry &extensionRegistry() override;
}
