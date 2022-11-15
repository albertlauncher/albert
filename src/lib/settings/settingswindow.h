// Copyright (c) 2014-2022 Manuel Schneider
#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
namespace albert { class ExtensionRegistry; }
class FrontendProvider;
class PluginProvider;
class TerminalProvider;

class SettingsWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(albert::ExtensionRegistry&);

    void bringToFront();

    Ui::SettingsWindow ui;

private:
    void keyPressEvent(QKeyEvent * event) override;

};
