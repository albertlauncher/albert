// Copyright (c) 2022-2024 Manuel Schneider

#include "logging.h"
#include "pluginmetadata.h"
#include "pluginregistry.h"
#include "pluginsmodel.h"
#include <QApplication>
#include <QPalette>
#include <QStyle>
using namespace albert;
using namespace std;


PluginsModel::PluginsModel(PluginRegistry &plugin_registry, QObject *parent):
    QAbstractListModel(parent),
    plugin_registry_(plugin_registry)
{
    for (auto &[_, plugin] : plugin_registry.plugins())
        plugins.emplace_back(&plugin);

    connect(&plugin_registry_, &PluginRegistry::pluginsChanged, this, [this] {
        beginResetModel();
        plugins.clear();
        for (auto &[id, plugin] : plugin_registry_.plugins())
        plugins.emplace_back(&plugin);
        endResetModel();
    });

    connect(&plugin_registry, &PluginRegistry::enabledChanged, this, [this](const QString &id) {
        if (auto it = ranges::find(plugins, id, &Plugin::id); it != plugins.end())
        {
            auto index = this->index(distance(begin(plugins), it));
            emit dataChanged(index, index, {Qt::CheckStateRole});
        }
        else
            WARN << "enabledChanged called for a plugin not in model: " << id;
    });

    connect(&plugin_registry, &PluginRegistry::stateChanged, this, [this](const QString &id) {
        if (auto it = ranges::find(plugins, id, &Plugin::id); it != plugins.end())
        {
            auto index = this->index(distance(begin(plugins), it));
            emit dataChanged(index, index, {Qt::DecorationRole,
                                            Qt::CheckStateRole,
                                            Qt::ForegroundRole,
                                            Qt::ToolTipRole});
        }
        else
            WARN << "stateChanged called for a plugin not in model: " << id;
    });
}

int PluginsModel::rowCount(const QModelIndex &) const
{ return static_cast<int>(plugins.size()); }

int PluginsModel::columnCount(const QModelIndex &) const
{ return 1; }

QVariant PluginsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    switch (const auto &p = *plugins[index.row()]; role) {

    case Qt::CheckStateRole:
        if (p.isUser())
        {
            if (p.state() == Plugin::State::Busy)
                return Qt::PartiallyChecked;
            else
                return p.isEnabled() ? Qt::Checked : Qt::Unchecked;
        }
        break;

    case Qt::DecorationRole:
        if (p.state() == Plugin::State::Unloaded && !p.stateInfo().isNull())
            return QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
        break;

    case Qt::DisplayRole:
        return p.metaData().name;

    case Qt::ForegroundRole:
        if (p.state() != Plugin::State::Loaded)
            return qApp->palette().color(QPalette::PlaceholderText);
        break;

    case Qt::ToolTipRole:
        return p.stateInfo();

    case Qt::UserRole:
        return p.id();

    }
    return {};
}

bool PluginsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole){
        try {
            if (auto &p = *plugins[index.row()]; p.isUser())
            {
                if (value == Qt::Checked)
                    plugin_registry_.enable(p.id());
                else if (value == Qt::Unchecked)
                    plugin_registry_.disable(p.id());
            }
        }
        catch (out_of_range &e){}
    }
    return false;
}

Qt::ItemFlags PluginsModel::flags(const QModelIndex &idx) const
{
    if (idx.isValid()){
        switch (auto &p = *plugins[idx.row()]; p.state())
        {
        case Plugin::State::Invalid:
            return Qt::ItemNeverHasChildren;
        case Plugin::State::Busy:
            return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        case Plugin::State::Loaded:
        case Plugin::State::Unloaded:
            return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        }
    }
    return Qt::NoItemFlags;
}
