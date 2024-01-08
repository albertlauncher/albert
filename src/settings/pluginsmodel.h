// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QAbstractListModel>
class PluginsModel;
class PluginRegistry;
namespace albert { class PluginLoader; }

class PluginsModel: public QAbstractListModel
{
public:
    explicit PluginsModel(PluginRegistry &plugin_registry);

    void updatePluginList();
    QIcon getCachedIcon(const QString &url) const;
    void updateView();

    // QAbstractListModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant&, int role) override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    std::vector<const albert::PluginLoader*> plugins_;
private:
    PluginRegistry &plugin_registry_;
    mutable std::map<QString, QIcon> icon_cache;
};
