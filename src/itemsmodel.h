// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QAbstractListModel>
#include <memory>
#include <vector>
#include <map>
class QueryBase;
namespace albert{
class Extension;
class Item;
class RankItem;
}

class ItemsModel final : public QAbstractListModel
{
public:
    ItemsModel(QObject *parent = nullptr); // important for qml cppownership

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void add(albert::Extension*, std::vector<std::shared_ptr<albert::Item>>&&);
    void add(std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator begin,
             std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator end);

    QAbstractListModel *buildActionsModel(uint i) const;

    void activate(QueryBase *q, uint i, uint a);

private:
    std::vector<std::pair<albert::Extension*, std::shared_ptr<albert::Item>>> items;
    mutable std::map<std::pair<albert::Extension*,albert::Item*>, QStringList> actionsCache;
};
