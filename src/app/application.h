// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "app.h"
#include <QObject>
#include <memory>
class FrontendRegistry;
class HotkeyManager;
class PathManager;
class PluginRegistry;
class QueryEngine;
class SystemTrayIcon;
class Localization;
class Telemetry;
namespace albert {
namespace detail { class Frontend; }
class ExtensionRegistry;
int run(int, char**);
}


class Application final : public albert::App
{
    Q_OBJECT

public:

    // Public interface
    void show(const QString &text = {}) override;
    void showSettings(QString plugin_id = {}) override;
    const std::map<QString, albert::Extension *> &extensions() const override;

    const std::filesystem::path &settingsFilePath() const;
    const std::filesystem::path &stateFilePath() const;

    void hide();
    void toggle();

    Localization &localization();
    PluginRegistry &pluginRegistry();
    FrontendRegistry &frontenRegistry();
    QueryEngine &queryEngine();
    Telemetry &telemetry();
    SystemTrayIcon &systemTrayIcon();
    PathManager &pathManager();
    HotkeyManager &hotkeyManager();

    static int run(const QStringList &additional_plugin_paths, bool load_enabled);

private:

    explicit Application(const QStringList &additional_plugin_paths, bool load_enabled);
    ~Application() override;

    friend int albert::run(int, char**);

    class Private;
    std::unique_ptr<Private> d;

};
