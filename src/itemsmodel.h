// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include "iconprovider.h"
#include <QAbstractListModel>
#include <QIcon>
#include <map>
#include <memory>
#include <vector>
namespace albert{ class Item; class RankItem; }

class ItemsModel : public QAbstractListModel
{
public:

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void add(albert::Extension*, const std::shared_ptr<albert::Item>&);
    void add(albert::Extension*, std::shared_ptr<albert::Item>&&);

    void add(albert::Extension*, const std::vector<std::shared_ptr<albert::Item>>&);
    void add(albert::Extension*, std::vector<std::shared_ptr<albert::Item>>&&);

    // For the global query
    void add(std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator begin,
             std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator end);

    std::vector<std::pair<albert::Extension*,std::shared_ptr<albert::Item>>> items;

    void clearCache();

private:
    static IconProvider icon_provider;
    static std::map<QString, QIcon> icon_cache;
};
