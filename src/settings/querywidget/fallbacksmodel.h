// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QAbstractTableModel>
#include <vector>
class QueryEngine;
namespace albert {
class FallbackHandler;
class Item;
}

class FallbacksModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    FallbacksModel(QueryEngine &qe, QObject *parent);

    void updateFallbackList();

private:

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int dstRow, int dstC, const QModelIndex &parent) override;
    bool moveRows(const QModelIndex &srcParent, int srcRow, int count,
                  const QModelIndex &dstParent, int dstRow) override;


    QueryEngine &engine;
    std::vector<std::pair<albert::FallbackHandler*, std::shared_ptr<albert::Item>>> fallbacks;

};
