// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QWidget>
class Plugin;
class PluginRegistry;
class QVBoxLayout;

class PluginWidget final : public QWidget
{
    Q_OBJECT

public:
    PluginWidget(const PluginRegistry &, const Plugin &);
    ~PluginWidget();
    void onPluginStateChanged(const QString &id);

private:
    QWidget *createPluginPageHeader() const;
    QWidget *createPluginPageBody() const;
    QWidget *createPluginPageFooter() const;

    const PluginRegistry &plugin_registry;
    const Plugin &plugin;
    QVBoxLayout *layout;
    QWidget *body;
};
