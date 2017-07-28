// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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
namespace Core {
class ExtensionManager;
}
class HotkeyManager;
class MainWindow;
class TrayIcon;

class SettingsWidget final : public QWidget
{
    Q_OBJECT

public:

    SettingsWidget(MainWindow *mainWindow,
                   HotkeyManager *hotkeyManager,
                   Core::ExtensionManager *extensionManager,
                   TrayIcon *trayIcon,
                   QWidget * parent = 0, Qt::WindowFlags f = 0);

private:

    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;
    void onThemeChanged(int);
    void onPluginDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void changeHotkey(int);
    void updatePluginInformations(const QModelIndex & curr);

    MainWindow *mainWindow_;
    HotkeyManager *hotkeyManager_;
    Core::ExtensionManager *extensionManager_;
    TrayIcon *trayIcon_;
    Ui::SettingsDialog ui;

};
