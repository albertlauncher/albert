// Copyright (c) 2022-2024 Manuel Schneider

#include "fallbackhandler.h"
#include "fallbacksmodel.h"
#include "iconprovider.h"
#include "queryengine.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QIODevice>
#include <QMimeData>
using namespace albert;
using namespace std;
using namespace util;

namespace {
enum class Column {
    Name,
    Description,
};
static int column_count = 2;
}

FallbacksModel::FallbacksModel(QueryEngine &e, QObject *p) : QAbstractTableModel(p), engine(e)
{
    connect(&e, &QueryEngine::handlerAdded, this, &FallbacksModel::updateFallbackList);
    connect(&e, &QueryEngine::handlerRemoved, this, &FallbacksModel::updateFallbackList);
    updateFallbackList();
}

void FallbacksModel::updateFallbackList()
{
    beginResetModel();

    fallbacks.clear();

    for (const auto &[hid, h] : engine.fallbackHandlers())
        for (auto f : h->fallbacks("…"))
            fallbacks.emplace_back(h, f);

    auto order = engine.fallbackOrder();

    ::sort(begin(fallbacks), end(fallbacks), [&](const auto &a, const auto &b)
           { return order[make_pair(a.first->id(), a.second->id())] > order[make_pair(b.first->id(), b.second->id())]; });

    endResetModel();
}

int FallbacksModel::rowCount(const QModelIndex &) const
{ return (int)fallbacks.size(); }

int FallbacksModel::columnCount(const QModelIndex &) const
{ return column_count; }

QVariant FallbacksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto &[h, i] = fallbacks[index.row()];
    auto c = (Column)index.column();

    if (c == Column::Name)
    {
        if (role == Qt::DecorationRole)
            try {
                return icon_cache.at(i->id());
            } catch (const out_of_range &) {
                return icon_cache[i->id()] = iconFromUrls(i->iconUrls());
            }

        else if (role == Qt::DisplayRole)
            return i->text();

        else if (role == Qt::ToolTipRole)
            return QString("%1 - %2").arg(h->id(), i->id());
    }

    else if (c == Column::Description)
    {
        if (role == Qt::DisplayRole)
            return i->subtext();
    }

    return {};
}

QVariant FallbacksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
        switch ((Column) section) {
        case Column::Name:        return tr("Name");
        case Column::Description: return tr("Description");
        }
    else if (role == Qt::ToolTipRole)
        switch ((Column) section) {
        case Column::Name:        return headerData(section, orientation, Qt::DisplayRole);
        case Column::Description: return headerData(section, orientation, Qt::DisplayRole);
        }
    return {};
}

Qt::ItemFlags FallbacksModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        switch ((Column) index.column())
        {
        case Column::Name:
        case Column::Description:
            return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
        }
    else
        return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;
    return {};
}

Qt::DropActions FallbacksModel::supportedDropActions() const { return Qt::MoveAction; }

bool FallbacksModel::dropMimeData(const QMimeData *data, Qt::DropAction, int dstRow, int, const QModelIndex &)
{
    QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    int srcRow = 0;
    if (!stream.atEnd())
        stream >> srcRow;
    return moveRows(QModelIndex(), srcRow, 1, QModelIndex(), dstRow);
}

bool FallbacksModel::moveRows(const QModelIndex &srcParent, int srcRow, int cnt, const QModelIndex &dstParent, int dstRow)
{
    if (srcRow < 0 || cnt < 1 || dstRow < 0)
        return false;

    // Exclude noop, segfaults
    if (srcRow <= dstRow && dstRow <= srcRow + cnt)
        return false;

    beginMoveRows(srcParent, srcRow, srcRow + cnt - 1, dstParent, dstRow);
    if (dstRow < srcRow)
        std::rotate(fallbacks.begin() + dstRow, fallbacks.begin() + srcRow, fallbacks.begin() + srcRow + cnt);
    else
        std::rotate(fallbacks.begin() + srcRow,fallbacks.begin() + srcRow + cnt, fallbacks.begin() + dstRow + cnt - 1);
    endMoveRows();

    map<pair<QString,QString>,int> newOrder;
    int rank = 1;
    for (auto it = fallbacks.rbegin(); it != fallbacks.rend(); ++it, ++rank)
        newOrder[make_pair(it->first->id(), it->second->id())] = rank;
    engine.setFallbackOrder(newOrder);

    return true;
}
