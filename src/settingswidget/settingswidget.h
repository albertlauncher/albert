// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "ui_settingswidget.h"
#include "hotkeymanager.h"
#include "mainwidget.h"
#include "pluginmanager.h"

class SettingsWidget final : public QWidget
{
    Q_OBJECT

public:
    SettingsWidget(MainWidget *mainWidget, HotkeyManager *hotkeyManager, PluginManager *pluginManager, QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~SettingsWidget();
    void show();

    Ui::SettingsDialog ui;

private:
    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;
    void updatePluginList();
    void updatePluginInformations();
    void changeHotkey(int);
    void onThemeChanged(int);
    void openPluginHelp();
    void openPluginConfig();
    void onPluginItemChanged(QTreeWidgetItem *item, int column);

    MainWidget *_mainWidget;
    HotkeyManager *_hotkeyManager;
    PluginManager *_pluginManager;
};
