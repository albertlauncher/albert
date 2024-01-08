// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/extensionwatcher.h"
#include <QAbstractTableModel>

class HandlerModel : public QAbstractTableModel,
                     public albert::ExtensionWatcher<albert::TriggerQueryHandler>,
                     public albert::ExtensionWatcher<albert::GlobalQueryHandler>,
                     public albert::ExtensionWatcher<albert::FallbackHandler>
{
    Q_OBJECT
public:
    explicit HandlerModel(QueryEngine &qe, albert::ExtensionRegistry &er, QObject *parent);

private:
    int rowCount(const QModelIndex &) const override;

    int columnCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &idx, int role) const override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    void addHandler(albert::Extension *handler);

    void removeHandler(albert::Extension *handler);

    inline void onAdd(albert::TriggerQueryHandler *handler) override { addHandler(handler); }

    inline void onAdd(albert::GlobalQueryHandler *handler) override { addHandler(handler); }

    inline void onAdd(albert::FallbackHandler *handler) override { addHandler(handler); }

    inline void onRem(albert::TriggerQueryHandler *handler) override { removeHandler(handler); }

    inline void onRem(albert::GlobalQueryHandler *handler) override { removeHandler(handler); }

    inline void onRem(albert::FallbackHandler *handler) override { removeHandler(handler); }

    std::vector<albert::Extension*> handlers;
    QueryEngine &engine;
    albert::ExtensionRegistry &registry;
};
