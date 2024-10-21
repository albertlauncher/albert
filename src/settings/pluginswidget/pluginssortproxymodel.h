// Copyright (c) 2024-2024 Manuel Schneider

#pragma once
#include <QSortFilterProxyModel>

class PluginsSortProxyModel : public QSortFilterProxyModel
{
    bool show_enabled_first_;

public:
    PluginsSortProxyModel(QObject *parent = nullptr);

    bool showEnabledFirst() const;
    void setShowEnabledFirst(bool);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

