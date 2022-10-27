// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extensionregistry.h"
#include <QString>
#include <map>

namespace albert
{

class ExtensionRegistry;
ExtensionRegistry &extensionRegistry();

class Query;
Query* query(const QString &query);

void show(const QString &text = QString());

void showSettings();

void restart();

void quit();

//void sendTrayNotification(const QString &title, const QString &message, const QIcon &icon) override;

//albert::Terminal &terminal() override;

//albert::ExtensionRegistry &extensionRegistry() override;
}
