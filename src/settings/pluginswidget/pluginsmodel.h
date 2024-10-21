// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QAbstractListModel>
class PluginRegistry;
class Plugin;

class PluginsModel: public QAbstractListModel
{
public:
    explicit PluginsModel(PluginRegistry &plugin_registry, QObject *parent = nullptr);

    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant&, int role) override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;

private:
    PluginRegistry &plugin_registry_;
    std::vector<const Plugin*> plugins;
};
