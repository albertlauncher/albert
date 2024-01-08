// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "pluginregistry.h"
#include "pluginsmodel.h"
#include <QIcon>
#include <QGuiApplication>
#include <QPalette>
using namespace std;
using namespace albert;


PluginsModel::PluginsModel(PluginRegistry &plugin_registry) : plugin_registry_(plugin_registry)
{
    connect(&plugin_registry, &PluginRegistry::pluginsChanged, this, &PluginsModel::updatePluginList);
    connect(&plugin_registry, &PluginRegistry::enabledChanged, this, &PluginsModel::updateView);
    updatePluginList();
}

QIcon PluginsModel::getCachedIcon(const QString &url) const
{
    try {
        return icon_cache.at(url);
    } catch (const std::out_of_range &e) {
        return icon_cache.emplace(url, url).first->second;
    }
}

int PluginsModel::rowCount(const QModelIndex &) const
{ return static_cast<int>(plugins_.size()); }

int PluginsModel::columnCount(const QModelIndex &) const
{ return 1; }

QVariant PluginsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return {};

    switch (const auto *p = plugins_[idx.row()]; role) {

    case Qt::CheckStateRole:
        if (p->metaData().load_type != LoadType::Frontend)
            switch (p->state()) {
            case PluginState::Busy:
                return Qt::PartiallyChecked;
            case PluginState::Loaded:
            case PluginState::Unloaded:
                return plugin_registry_.isEnabled(p->metaData().id) ? Qt::Checked : Qt::Unchecked;
            }
        break;

    case Qt::DisplayRole:
        return p->metaData().name;

    case Qt::ForegroundRole:
        if (p->state() != PluginState::Loaded)
            return qApp->palette().color(QPalette::PlaceholderText);
        if (!p->stateInfo().isNull())
            return QColor(Qt::red);
        break;

    case Qt::ToolTipRole:
        return p->stateInfo();

    case Qt::UserRole:
        return p->metaData().id;

    }
    return {};
}

bool PluginsModel::setData(const QModelIndex &idx, const QVariant &, int role)
{
    if (idx.isValid() && idx.column() == 0 && role == Qt::CheckStateRole){
        try {
            const auto *p = plugins_[idx.row()];
            if (p->metaData().load_type != LoadType::Frontend && (p->state() == PluginState::Loaded || p->state() == PluginState::Unloaded))
                plugin_registry_.enable(p->metaData().id, !plugin_registry_.isEnabled(p->metaData().id));
        } catch (std::out_of_range &e){}
    }
    return false;
}

Qt::ItemFlags PluginsModel::flags(const QModelIndex &idx) const
{
    if (idx.isValid()){
        switch (plugins_[idx.row()]->state()) {  // TODO: if
        case PluginState::Busy:
            return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        case PluginState::Loaded:
        case PluginState::Unloaded:
            return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        }
    }
    return Qt::NoItemFlags;
}

void PluginsModel::updatePluginList()
{
    beginResetModel();

    plugins_.clear();
    for (const auto &[id, loader] : plugin_registry_.plugins()){
        plugins_.emplace_back(loader);
        connect(loader, &PluginLoader::stateChanged, this, &PluginsModel::updateView, Qt::UniqueConnection);
    }

    ::sort(plugins_.begin(), plugins_.end(),
           [](const auto &l, const auto &r){ return l->metaData().name < r->metaData().name; });

    endResetModel();
}

void PluginsModel::updateView()
{
    emit dataChanged(index(0), index(plugins_.size()-1));
}
