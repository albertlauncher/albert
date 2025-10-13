// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
class PluginRegistry;
class QueryEngine;
class QHotkey;
class Telemetry;
namespace albert {
namespace detail { class Frontend; }
class ExtensionRegistry;
int run(int, char**);
}


class App : public QObject
{
    Q_OBJECT

public:

    static App *instance();

    void show(const QString &text = {});
    void hide();
    void toggle();
    void restart();
    void quit();
    Q_INVOKABLE void handleUrl(const QUrl &url);

    albert::ExtensionRegistry &extensionRegistry();
    PluginRegistry &pluginRegistry();
    QueryEngine &queryEngine();
    Telemetry &telemetry();

    void showSettings(QString plugin_id = {});

    bool trayEnabled() const;
    void setTrayEnabled(bool);

    const QStringList &originalPathEntries() const;
    const QStringList &additionalPathEntries() const;
    void setAdditionalPathEntries(const QStringList&);

    const QHotkey *hotkey() const;
    void setHotkey(std::unique_ptr<QHotkey> hotkey);

    QStringList availableFrontends();
    QString currentFrontend();
    void setFrontend(uint i);
    albert::detail::Frontend *frontend();

private:

    explicit App(const QStringList &additional_plugin_paths, bool load_enabled);
    ~App() override;

    void initialize();
    void finalize();

    friend int albert::run(int, char**);

    class Private;
    std::unique_ptr<Private> d;

};
