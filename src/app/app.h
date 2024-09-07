// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
class PluginRegistry;
class QueryEngine;
class QHotkey;
namespace albert {
class Frontend;
int run(int, char**);
}


class App : public QObject
{
    Q_OBJECT

public:

    static App *instance();

    void show(const QString &text);
    void hide();
    void toggle();
    void restart();
    void quit();

    PluginRegistry &pluginRegistry();
    QueryEngine &queryEngine();

    void showSettings(QString plugin_id = {});

    bool trayEnabled() const;
    void setTrayEnabled(bool);

    bool telemetryEnabled() const;
    void setTelemetryEnabled(bool);
    QString displayableTelemetryReport() const;

    const QHotkey *hotkey() const;
    void setHotkey(std::unique_ptr<QHotkey> hotkey);

    QStringList availableFrontends();
    QString currentFrontend();
    void setFrontend(uint i);
    albert::Frontend *frontend();

private:

    explicit App(const QStringList &additional_plugin_paths, bool load_enabled);
    ~App() override;

    void initialize();
    void finalize();

    friend int albert::run(int, char**);

    class Private;
    std::unique_ptr<Private> d;

};
