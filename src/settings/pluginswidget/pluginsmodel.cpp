// Copyright (c) 2022-2025 Manuel Schneider

#include "logging.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginregistry.h"
#include "pluginsmodel.h"
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <ranges>
using enum Plugin::State;
using enum albert::PluginMetadata::LoadType;
using namespace Qt;
using namespace albert;
using namespace std;


PluginsModel::PluginsModel(PluginRegistry &plugin_registry, QObject *parent):
    QAbstractListModel(parent),
    plugin_registry_(plugin_registry)
{
    for (auto &[_, plugin] : plugin_registry.plugins())
        plugins.emplace_back(&plugin);

    connect(&plugin_registry_, &PluginRegistry::pluginsChanged,
            this, [this]
            {
                auto v = plugin_registry_.plugins()
                         | views::transform([](auto &p){ return &p.second; });
                vector<const Plugin*> vec(v.begin(), v.end());  // ranges::to
                ranges::sort(vec, less<>{}, [](auto p){ return p->id; });
                beginResetModel();
                plugins = std::move(vec);
                endResetModel();
            });

    connect(&plugin_registry, &PluginRegistry::pluginEnabledChanged,
            this, [this](const QString &id)
            {
                if (auto it = ranges::find(plugins, id, [](auto p){ return p->id; });
                    it != plugins.end())
                {
                    auto index = this->index(distance(begin(plugins), it));
                    emit dataChanged(index, index, {CheckStateRole});
                }
                else
                    WARN << "enabledChanged called for a plugin not in model: " << id;
            });

    connect(&plugin_registry, &PluginRegistry::pluginStateChanged,
            this, [this](const QString &id)
            {
                if (auto it = ranges::find(plugins, id, [](auto p){ return p->id; });
                    it != plugins.end())
                {
                    auto index = this->index(distance(begin(plugins), it));
                    emit dataChanged(index, index, {DecorationRole,
                                                    CheckStateRole,
                                                    ForegroundRole,
                                                    ToolTipRole});
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

    switch (const auto &p = *plugins[index.row()];
            role)
    {
    case CheckStateRole:
        if (p.metadata.load_type == User)
        {
            if (p.state == Loading)
                return PartiallyChecked;
            else
                return p.enabled ? Checked : Unchecked;
        }
        break;

    case DecorationRole:
        if (p.state == Unloaded && !p.state_info.isNull())
            return QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
        break;

    case DisplayRole:
        return p.metadata.name;

    case ForegroundRole:
        if (p.state != Loaded)
            return qApp->palette().color(QPalette::PlaceholderText);
        break;

    case ToolTipRole:
        return p.state_info;

    case UserRole:
        return p.id;

    }
    return {};
}

bool PluginsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && index.column() == 0 && role == CheckStateRole)
    {
        try
        {
            if (auto &p = *plugins[index.row()];
                p.metadata.load_type == User)
            {
                if (value == Checked)
                    plugin_registry_.setEnabledWithUserConfirmation(p.id, true);
                else if (value == Unchecked)
                    plugin_registry_.setEnabledWithUserConfirmation(p.id, false);
            }
        }
        catch (out_of_range &e){}
    }
    return false;
}

ItemFlags PluginsModel::flags(const QModelIndex &idx) const
{
    if (idx.isValid())
    {
        switch (auto &p = *plugins[idx.row()];
                p.state)
        {
        case Loading:
            return ItemNeverHasChildren | ItemIsSelectable | ItemIsEnabled;
        case Loaded:
        case Unloaded:
            return ItemNeverHasChildren | ItemIsSelectable | ItemIsEnabled | ItemIsUserCheckable;
        }
    }
    return NoItemFlags;
}
