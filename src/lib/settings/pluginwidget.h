// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QTableView>
class PluginModel;
class PluginRegistry;

class PluginWidget final : public QTableView
{
    Q_OBJECT
public:
    explicit PluginWidget(PluginRegistry&);
    ~PluginWidget();
private:
    std::unique_ptr<PluginModel> model_;
};

