// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensionwatcher.h"
#include <QAbstractTableModel>
#include <QListWidget>
#include <QStackedWidget>
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
    void resetUI();

protected:
    void onAdd(albert::ConfigWidgetProvider *t) override;
    void onRem(albert::ConfigWidgetProvider *t) override;

private:

    albert::ExtensionRegistry &registry;
    QListWidget list_widget;
    QStackedWidget stacked_widget;

};

