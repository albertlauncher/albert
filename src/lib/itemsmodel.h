// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "iconprovider.h"
#include <QAbstractListModel>
#include <memory>
#include <vector>
namespace albert{ class Item; class RankItem; }

class ItemsModel : public QAbstractListModel
{
public:

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void add(albert::QueryHandler *handler, const std::shared_ptr<albert::Item> &item);
    void add(albert::QueryHandler *handler, std::shared_ptr<albert::Item> &&item);

    void add(albert::QueryHandler *handler, const std::vector<std::shared_ptr<albert::Item>> &items);
    void add(albert::QueryHandler *handler, std::vector<std::shared_ptr<albert::Item>> &&items);

    // For the global query
    void add(std::vector<std::pair<albert::QueryHandler*,albert::RankItem>>::iterator begin,
             std::vector<std::pair<albert::QueryHandler*,albert::RankItem>>::iterator end);

    std::vector<std::pair<albert::QueryHandler*,std::shared_ptr<albert::Item>>> items;

private:
    static IconProvider icon_provider;
};
