// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QWidget>
#include <memory>
class QListView;
class QScrollArea;
class PluginsModel;
class PluginRegistry;

class PluginsWidget final : public QWidget
{
    Q_OBJECT

public:

    PluginsWidget(PluginRegistry&);
    ~PluginsWidget();

    void tryShowPluginSettings(QString);

private:

    void onUpdatePluginWidget();
    void updatePluginListWidth();

    std::unique_ptr<PluginsModel> model_;
    QListView *listView_plugins;
    QScrollArea *scrollArea_info;
};

