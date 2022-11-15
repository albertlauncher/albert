// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/util/extensionwatcher.h"
#include <QTreeView>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <vector>
namespace albert {
class ExtensionRegistry;
class ConfigWidgetProvider;
}

class SettingsWidget final :
        public QWidget,
        public albert::ExtensionWatcher<albert::ConfigWidgetProvider>
{
    Q_OBJECT

public:
    explicit SettingsWidget(albert::ExtensionRegistry&registry);

private:
    void onAdd(albert::ConfigWidgetProvider *t) override;
    void onRem(albert::ConfigWidgetProvider *t) override;
    void resetTreeModel();

private:

    albert::ExtensionRegistry &registry;
    std::map<QString,albert::ConfigWidgetProvider*> config_providers;
    QTreeView tree_view;
    QStandardItemModel tree_model;
    QStackedWidget stacked_widget;



};

