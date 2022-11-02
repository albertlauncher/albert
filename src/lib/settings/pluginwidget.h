// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensionwatcher.h"
#include <QAbstractTableModel>
#include <QTableView>
#include <map>
namespace albert {
class PluginSpec;
class PluginProvider;
class Extension;
}

struct PluginInfoWidget : public QWidget
{
    PluginInfoWidget(const albert::PluginSpec &spec);
};


struct PluginModel :
        public QAbstractTableModel,
        public albert::ExtensionWatcher<albert::PluginProvider>
{
    PluginModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    void onAdd(albert::PluginProvider *t) override;
    void onRem(albert::PluginProvider *t) override;

    std::vector<const albert::PluginSpec*> plugins;
    void updatePlugins();
    void onPluginStateChanged(const albert::PluginSpec &spec);
};

struct PluginWidget final : public QTableView
{
    PluginWidget();
    PluginModel model;
    Q_OBJECT
};

