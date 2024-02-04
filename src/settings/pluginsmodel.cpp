// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "pluginregistry.h"
#include "pluginsmodel.h"
#include <QIcon>
#include <QPalette>
#include <QStyle>
#include <QApplication>
using namespace std;
using namespace albert;

PluginsModel::PluginsModel(PluginRegistry &plugin_registry) : plugin_registry_(plugin_registry)
{
    connect(&plugin_registry, &PluginRegistry::pluginsChanged, this, &PluginsModel::updatePluginList);
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

QVariant PluginsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    switch (const auto &p = *plugins_[index.row()]; role) {

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
            if (auto &p = *plugins_[index.row()]; p.isUser())
            {
                if (value == Qt::Checked)
                    plugin_registry_.enable(p.id());
                else if (value == Qt::Unchecked)
                    plugin_registry_.disable(p.id());
            }
        }
        catch (std::out_of_range &e){}
    }
    return false;
}

Qt::ItemFlags PluginsModel::flags(const QModelIndex &idx) const
{;
    if (idx.isValid()){
        switch (auto &p = *plugins_[idx.row()]; p.state())
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

void PluginsModel::updatePluginList()
{
    beginResetModel();

    plugins_.clear();

    for (auto &[id, loader] : plugin_registry_.plugins()){
        plugins_.emplace_back(&loader);
        connect(&loader, &Plugin::stateChanged, this, &PluginsModel::updateView, Qt::UniqueConnection);
        connect(&loader, &Plugin::enabledChanged, this, &PluginsModel::updateView, Qt::UniqueConnection);
    }

    ::sort(plugins_.begin(), plugins_.end(),
           [](const auto &l, const auto &r){ return l->metaData().name < r->metaData().name; });

    endResetModel();
}

void PluginsModel::updateView()
{
    // Well not worth the optimizations
    emit dataChanged(index(0), index(plugins_.size()-1));
}
