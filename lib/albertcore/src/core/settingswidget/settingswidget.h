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

namespace GlobalShortcut {
class HotkeyManager;
}
namespace Core {
class ExtensionManager;
class FrontendManager;
class QueryManager;
class MainWindow;
class TrayIcon;
class Telemetry;

class SettingsWidget final : public QWidget
{
    Q_OBJECT

public:

    SettingsWidget(ExtensionManager *extensionManager,
                   FrontendManager *frontendManager,
                   QueryManager *queryManager,
                   GlobalShortcut::HotkeyManager *hotkeyManager,
                   TrayIcon *trayIcon,
                   Telemetry *telemetry,
                   QWidget * parent = nullptr,
                   Qt::WindowFlags f = nullptr);

private:

    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;
    void onPluginDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void changeHotkey(int);
    void updatePluginInformations(const QModelIndex & curr);

    ExtensionManager *extensionManager_;
    FrontendManager *frontendManager_;
    QueryManager *queryManager_;
    GlobalShortcut::HotkeyManager *hotkeyManager_;
    TrayIcon *trayIcon_;
    Telemetry *telemetry_;
    Ui::SettingsDialog ui;

};

}
