// Copyright (c) 2022-2024 Manuel Schneider

#include "globalqueryhandler.h"
#include "queryengine.h"
#include "queryhandlermodel.h"
#include "triggerqueryhandler.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <set>
using namespace albert;
using namespace std;

namespace {
enum class Column { Name, Trigger, Global, Fuzzy };
static int column_count = 4;
}


QueryHandlerModel::QueryHandlerModel(QueryEngine &qe, QObject *parent)
    : QAbstractTableModel(parent), engine(qe)
{
    connect(&engine, &QueryEngine::handlerAdded, this, &QueryHandlerModel::updateHandlers);
    connect(&engine, &QueryEngine::handlerRemoved, this, &QueryHandlerModel::updateHandlers);
    updateHandlers();
}

void QueryHandlerModel::updateHandlers()
{
    beginResetModel();

    handlers_.clear();
    for (auto &[id, h] : engine.triggerHandlers())
        handlers_.emplace_back(h);

    ::sort(begin(handlers_), end(handlers_),
           [&](auto *a, auto *b){ return a->name() < b->name(); });

    endResetModel();
}

int QueryHandlerModel::rowCount(const QModelIndex&) const
{ return handlers_.size(); }

int QueryHandlerModel::columnCount(const QModelIndex&) const
{ return column_count; }

QVariant QueryHandlerModel::data(const QModelIndex &idx, int role) const
{
    const auto *h = handlers_[idx.row()];

    if (idx.column() == (int) Column::Name)
    {
        if (role == Qt::DisplayRole)
            return h->name();

        else if (role == Qt::ToolTipRole)
            return h->description();
    }

    else if (idx.column() == (int) Column::Trigger)
    {
        auto t = engine.trigger(h->id());

        if (role == Qt::DisplayRole)
            return t.replace(" ", "•");

        else if (role == Qt::EditRole)
            return t;

        else if (role == Qt::ToolTipRole)
        {
            if (!h->allowTriggerRemap())
                return tr("This extension does not allow trigger remapping.");
            else if (auto it = engine.activeTriggerHandlers().find(t);
                     it != engine.activeTriggerHandlers().end() && it->second != h)
                return tr("Trigger '%1' is reserved for '%2'.")
                    .arg(t, it->second->name());
        }

        else if (role == Qt::ForegroundRole)
        {
            if (auto it = engine.activeTriggerHandlers().find(t);
                it == engine.activeTriggerHandlers().end() || it->second != h)
                return QColor(Qt::red);
            else if (!h->allowTriggerRemap())
                return QColor(Qt::gray);
        }
    }

    else if (idx.column() == (int) Column::Global)
    {
        if (auto *gh = dynamic_cast<const GlobalQueryHandler*>(h); gh)
        {
            if (role == Qt::CheckStateRole)
                return engine.isEnabled(gh->id()) ? Qt::Checked : Qt::Unchecked;

            else if (role == Qt::ToolTipRole)
            {
                if (engine.isEnabled(gh->id()))
                    return tr("Disable global query handler.");
                else
                    return tr("Enable global query handler.");
            }
        }
    }

    else if (idx.column() == (int) Column::Fuzzy)
    {
        if (h->supportsFuzzyMatching())
        {
            if (role == Qt::CheckStateRole)
                return engine.fuzzy(h->id()) ? Qt::Checked : Qt::Unchecked;

            else if (role == Qt::ToolTipRole)
            {
                if (engine.fuzzy(h->id()))
                    return tr("Disable fuzzy matching.");
                else
                    return tr("Enable fuzzy matching.");
            }
        }
    }

    return {};
}

bool QueryHandlerModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    auto *h = handlers_[idx.row()];

    if (idx.column() == (int) Column::Trigger)
    {
        if (role == Qt::EditRole)
        {
            if (const auto it = engine.activeTriggerHandlers().find(value.toString());
                it != engine.activeTriggerHandlers().end() && it->second != h)
                QMessageBox::warning(nullptr, qApp->applicationName(),
                                     tr("Trigger '%1' is reserved for '%2'.")
                                         .arg(value.toString(), it->second->name()));
            else
            {
                engine.setTrigger(h->id(), value.toString());
                emit dataChanged(index(idx.row(), idx.column()+1), idx, {Qt::DisplayRole});
                return true;
            }
        }
    }

    else if (idx.column() == (int) Column::Global)
    {
        if (auto *gh = dynamic_cast<GlobalQueryHandler*>(h); gh)
        {
            if (role == Qt::CheckStateRole)
            {
                engine.setEnabled(gh->id(), value == Qt::Checked);
                return true;
            }
        }
    }

    else if (idx.column() == (int) Column::Fuzzy)
    {
        if (role == Qt::CheckStateRole) {
            engine.setFuzzy(h->id(), value == Qt::Checked);
            return true;
        }
    }

    return false;
}

QVariant QueryHandlerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
        switch ((Column) section) {
        case Column::Name: return tr("Extension");
        case Column::Trigger: return tr("Trigger");
        case Column::Global: return tr("G", "short Global");
        case Column::Fuzzy: return tr("F", "short Fuzzy");
        }
    else if (role == Qt::ToolTipRole)
        switch ((Column) section) {
        case Column::Name: return headerData(section, orientation, Qt::DisplayRole);
        case Column::Trigger: return tr("The trigger of the handler. Spaces are visualized by •.");
        case Column::Global: return tr("Enabled global query handlers.");
        case Column::Fuzzy: return tr("Fuzzy matching.");
        }
    return {};
}

Qt::ItemFlags QueryHandlerModel::flags(const QModelIndex &idx) const
{
    auto *h = handlers_[idx.row()];

    switch ((Column) idx.column()) {
    case Column::Name:
        return Qt::NoItemFlags;
    case Column::Trigger:
        return h->allowTriggerRemap() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable : Qt::NoItemFlags;
    case Column::Global:
        return dynamic_cast<GlobalQueryHandler*>(h) ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable : Qt::NoItemFlags;
    case Column::Fuzzy:
        return h->supportsFuzzyMatching() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable : Qt::NoItemFlags;
    }
    return {};
}
