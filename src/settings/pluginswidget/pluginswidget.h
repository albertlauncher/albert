// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QWidget>
class PluginRegistry;
class PluginsModel;
class PluginsSortProxyModel;
class QListView;
class QScrollArea;

class PluginsWidget final : public QWidget
{
    Q_OBJECT

public:
    PluginsWidget(PluginRegistry&);
    ~PluginsWidget();
    void tryShowPluginSettings(QString);

private:
    void setPlaceholderWidget();
    void showContextMenu(const QPoint &pos);
    void updatePluginListWidth();

    PluginRegistry &plugin_registry_;

    PluginsModel *model_;
    PluginsSortProxyModel *proxy_model_;

    QListView *plugins_list_view_;
    QScrollArea *config_widget_scroll_area_;
};
