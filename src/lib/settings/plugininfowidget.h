// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/util/extensionwatcher.h"
#include <QAbstractTableModel>
#include <QTableView>
#include <map>
#include <set>
namespace albert {
class PluginSpec;
class PluginProvider;
class Extension;
}

struct PluginInfoWidget : public QWidget
{
    explicit PluginInfoWidget(const albert::PluginSpec &spec);
};


struct PluginModel :
        public QAbstractTableModel,
        public albert::ExtensionWatcher<albert::PluginProvider>
{
    explicit PluginModel(albert::ExtensionRegistry &);
    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int role) const override;
    bool setData(const QModelIndex&, const QVariant&, int) override;
    QVariant headerData(int, Qt::Orientation, int) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;

    void onAdd(albert::PluginProvider *t) override;
    void onRem(albert::PluginProvider *t) override;

    std::vector<const albert::PluginSpec*> plugins;
    void updatePlugins();
};

struct PluginWidget final : public QTableView
{
    explicit PluginWidget(albert::ExtensionRegistry&);
    PluginModel model;
    Q_OBJECT
};

