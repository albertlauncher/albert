// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QAbstractListModel>
#include <vector>
#include <memory>
namespace albert{ class Item; }

class ItemsModel : public QAbstractListModel
{
public:

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void add(const std::shared_ptr<albert::Item> &item);
    void add(std::shared_ptr<albert::Item> &&item);
    void add(const std::vector<std::shared_ptr<albert::Item>> &items);
    void add(std::vector<std::shared_ptr<albert::Item>> &&items);

    std::vector<std::shared_ptr<albert::Item>> items;
};
