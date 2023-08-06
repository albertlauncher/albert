// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include <QWidget>
#include <QListView>
#include <QScrollArea>
class PluginModel;
class PluginRegistry;

class PluginWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit PluginWidget(PluginRegistry&);
    ~PluginWidget();

    void tryShowPluginSettings(QString);

private:
    void onUpdatePluginWidget();

    std::unique_ptr<PluginModel> model_;
    QListView *listView_plugins;
    QScrollArea *scrollArea_info;
};

