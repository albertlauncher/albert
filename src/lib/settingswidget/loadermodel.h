// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QAbstractListModel>

namespace Core {

class ExtensionManager;

class LoaderModel final : public QAbstractListModel
{
public:
    LoaderModel(Core::ExtensionManager* pm, QObject *parent = nullptr);
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
private:
    Core::ExtensionManager *extensionManager_;
};

}
