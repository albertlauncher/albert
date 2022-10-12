// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include "ui_settingswidget.h"

namespace Core {
class ExtensionManager;
class FrontendManager;
class QueryManager;
class MainWindow;
class TrayIcon;

class SettingsWidget final : public QWidget
{
    Q_OBJECT

public:

    SettingsWidget(ExtensionManager *extensionManager,
                   FrontendManager *frontendManager,
                   QueryManager *queryManager,
                   TrayIcon *trayIcon,
                   QWidget * parent = nullptr,
                   Qt::WindowFlags f = nullptr);

private:

    void keyPressEvent(QKeyEvent * event) override;
    void onPluginDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void updatePluginInformations(const QModelIndex & curr);

    ExtensionManager *extensionManager_;
    FrontendManager *frontendManager_;
    QueryManager *queryManager_;
    TrayIcon *trayIcon_;
    Ui::SettingsDialog ui;

};

}
