// Copyright (c) 2022-2024 Manuel Schneider

#include "queryengine.h"
#include "queryhandlermodel.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <set>
using namespace albert;
using namespace std;

namespace {
enum class Column {
    Name,
    Trigger,
    THandler,
    GHandler,
    Fuzzy,
};
static int column_count = 5;
}


QueryHandlerModel::QueryHandlerModel(QueryEngine &qe, QObject *parent)
    : QAbstractTableModel(parent), engine(qe)
{
    connect(&engine, &QueryEngine::handlersChanged,
            this, &QueryHandlerModel::updateHandlers);

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
        if (role == Qt::DisplayRole)
            return h->trigger().replace(" ", "•");

        else if (role == Qt::EditRole)
            return h->trigger();

        else if (role == Qt::ToolTipRole)
        {
            if (!h->allowTriggerRemap())
                return tr("This extension does not allow trigger remapping.");
            else if (auto it = engine.activeTriggerHandlers().find(h->trigger());
                     it != engine.activeTriggerHandlers().end() && it->second != h)
                return tr("Trigger '%1' is reserved for '%2'.")
                    .arg(h->trigger(), it->second->name());
        }

        else if (role == Qt::FontRole && !h->allowTriggerRemap())
        {
            QFont f;
            f.setItalic(true);
            return f;
        }

        else if (role == Qt::ForegroundRole)
        {
            if (!engine.isEnabled(h))
                return QColor(Qt::gray);
            else if (auto it = engine.activeTriggerHandlers().find(h->trigger());
                     it == engine.activeTriggerHandlers().end() || it->second != h)
                return QColor(Qt::red);
        }
    }

    else if (idx.column() == (int) Column::THandler)
    {
        if (role == Qt::CheckStateRole)
            return engine.isEnabled(h) ? Qt::Checked : Qt::Unchecked;

        else if (role == Qt::ToolTipRole)
        {
            if (engine.isEnabled(h))
                return tr("Disable trigger query handler.");
            else
                return tr("Enable trigger query handler.");
        }
    }

    else if (idx.column() == (int) Column::GHandler)
    {
        if (auto *gh = dynamic_cast<const GlobalQueryHandler*>(h); gh)
        {
            if (role == Qt::CheckStateRole)
                return engine.isEnabled(gh) ? Qt::Checked : Qt::Unchecked;

            else if (role == Qt::ToolTipRole)
            {
                if (engine.isEnabled(gh))
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
                return engine.fuzzy(h) ? Qt::Checked : Qt::Unchecked;

            else if (role == Qt::ToolTipRole)
            {
                if (engine.fuzzy(h))
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
                engine.setTrigger(h, value.toString());
                emit dataChanged(index(idx.row(), idx.column()+1), idx, {Qt::DisplayRole});
                return true;
            }
        }
    }

    else if (idx.column() == (int)Column::THandler)
    {
        if (role == Qt::CheckStateRole)
        {
            engine.setEnabled(h, value == Qt::Checked);
            emit dataChanged(index(idx.row(), idx.column()-1), idx, {Qt::DisplayRole});
            return true;
        }
    }

    else if (idx.column() == (int) Column::GHandler)
    {
        if (auto *gh = dynamic_cast<GlobalQueryHandler*>(h); gh)
        {
            if (role == Qt::CheckStateRole)
            {
                engine.setEnabled(gh, value == Qt::Checked);
                return true;
            }
        }
    }

    else if (idx.column() == (int) Column::Fuzzy)
    {
        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(h); thandler){
            if (role == Qt::CheckStateRole) {
                engine.setFuzzy(thandler, value == Qt::Checked);
                return true;
            }
        }
    }

    return false;
}

QVariant QueryHandlerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
        switch ((Column) section) {
        case Column::Name:        return tr("Extension");
        case Column::Trigger:     return tr("Trigger");
        case Column::THandler:    return tr("T", "short Trigger");
        case Column::GHandler:    return tr("G", "short Global");
        case Column::Fuzzy:       return tr("F", "short Fuzzy");
        }
    else if (role == Qt::ToolTipRole)
        switch ((Column) section) {
        case Column::Name:        return headerData(section, orientation, Qt::DisplayRole);
        case Column::Trigger:     return tr("The trigger of the handler. Spaces are visualized by •.");
        case Column::THandler:    return tr("Enabled trigger query handlers.");
        case Column::GHandler:    return tr("Enabled global query handlers.");
        case Column::Fuzzy:       return tr("Fuzzy matching.");
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
    case Column::THandler:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    case Column::GHandler:
        return dynamic_cast<GlobalQueryHandler*>(h) ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable : Qt::NoItemFlags;
    case Column::Fuzzy:
        return h->supportsFuzzyMatching() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable : Qt::NoItemFlags;
    }
    return {};
}
