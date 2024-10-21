// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QWidget>
class Plugin;
class QVBoxLayout;

class PluginWidget final : public QWidget
{
public:
    PluginWidget(const Plugin &);
    ~PluginWidget();
    void onPluginStateChanged();

private:
    QWidget *createPluginPageBody();
    QWidget *createPluginPageFooter();
    QWidget *createPluginPageHeader();

    const Plugin &plugin;
    QVBoxLayout *layout;
    QWidget *body;
};
