// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QAbstractTableModel>
class QueryEngine;
namespace albert { class TriggerQueryHandler; }

class QueryHandlerModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit QueryHandlerModel(QueryEngine &qe, QObject *parent);

private:

    void updateHandlers();

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    QueryEngine &engine;
    std::vector<albert::TriggerQueryHandler*> handlers_;

};
