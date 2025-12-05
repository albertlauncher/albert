// Copyright (c) 2024-2024 Manuel Schneider

#include "albert/app.h"
#include "pluginssortproxymodel.h"
#include <QSettings>
using namespace albert;
const char* CFG_SORT_MODE = "show_enabled_plugins_first";
const bool  DEF_SORT_MODE = true;


PluginsSortProxyModel::PluginsSortProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    show_enabled_first_ = App::settings()->value(CFG_SORT_MODE, DEF_SORT_MODE).toBool();
}

bool PluginsSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (show_enabled_first_)
    {
        auto l = left.data(Qt::CheckStateRole).toInt();
        auto r = right.data(Qt::CheckStateRole).toInt();
        if (l != r)
            return l > r;
    }
    return left.data(Qt::DisplayRole).toString() < right.data(Qt::DisplayRole).toString();
}

bool PluginsSortProxyModel::showEnabledFirst() const { return show_enabled_first_; }

void PluginsSortProxyModel::setShowEnabledFirst(bool value)
{
    if (value != show_enabled_first_)
    {
        show_enabled_first_ = value;
        App::settings()->setValue(CFG_SORT_MODE, show_enabled_first_);
        invalidate();
        sort(0);
    }
}

