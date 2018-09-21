// Copyright (C) 2014-2018 Manuel Schneider

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
