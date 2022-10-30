// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QAbstractTableModel>
#include <QListWidget>
#include <QStackedWidget>
#include "extensionwatcher.h"
#include "settingswidgetprovider.h"
#include <vector>
class App;

class SettingsWidget final :
        public QWidget,
        public albert::ExtensionWatcher<albert::SettingsWidgetProvider>
{
    Q_OBJECT

public:
    SettingsWidget(App &albert);

private:
    void resetUI();

protected:
    void onAdd(albert::SettingsWidgetProvider *t) override;
    void onRem(albert::SettingsWidgetProvider *t) override;

private:

    App &app;
    QListWidget list_widget;
    QStackedWidget stacked_widget;

};

