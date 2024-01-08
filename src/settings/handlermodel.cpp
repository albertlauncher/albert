// Copyright (c) 2022-2023 Manuel Schneider

#include "handlermodel.h"
#include "queryengine.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <set>
using namespace albert;
using namespace std;

enum class Column {
    Name,
    Trigger,
    THandler,
    GHandler,
    FHandler,
    Fuzzy,
    Description,
};

HandlerModel::HandlerModel(QueryEngine &qe, albert::ExtensionRegistry &er, QObject *parent):
    QAbstractTableModel(parent),
    ExtensionWatcher<TriggerQueryHandler>(&er),
    ExtensionWatcher<GlobalQueryHandler>(&er),
    ExtensionWatcher<FallbackHandler>(&er),
    engine(qe), registry(er)
{
    set<Extension*> unique_handlers;
    for (auto &[id, handler]: registry.extensions<TriggerQueryHandler>()) unique_handlers.emplace(handler);
    for (auto &[id, handler]: registry.extensions<GlobalQueryHandler>()) unique_handlers.emplace(handler);
    for (auto &[id, handler]: registry.extensions<FallbackHandler>()) unique_handlers.emplace(handler);
    handlers = vector<Extension*>{unique_handlers.begin(), unique_handlers.end()};
    ::sort(begin(handlers), end(handlers), [](const auto &a, const auto &b) { return a->name() < b->name(); });
}

int HandlerModel::rowCount(const QModelIndex &) const { return (int)handlers.size(); }

int HandlerModel::columnCount(const QModelIndex &) const { return 7; }

QVariant HandlerModel::data(const QModelIndex &idx, int role) const
{
    auto &handler = handlers[idx.row()];

    if (idx.column() == (int) Column::Name) {

        if (role == Qt::DisplayRole)
            return handler->name();

        else if (role == Qt::ToolTipRole)
            return handler->id();

    } else if (idx.column() == (int) Column::Trigger) {

        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){

            if (role == Qt::DisplayRole) {
                return thandler->trigger().replace(" ", "•");

            } else if (role == Qt::EditRole) {
                return thandler->trigger();

            } else if (role == Qt::ToolTipRole) {
                if (!thandler->allowTriggerRemap())
                    return tr("This extension does not allow trigger remapping.");

            } else if (role == Qt::FontRole && !thandler->allowTriggerRemap()) {
                QFont f;
                f.setItalic(true);
                return f;

            } else if (role == Qt::ForegroundRole) {
                if (!engine.isActive(thandler))
                    return QColor(Qt::gray);
            }
        }

    } else if (idx.column() == (int) Column::THandler) {

        if (role == Qt::CheckStateRole) {
            if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler)
                if (role == Qt::CheckStateRole)
                    return engine.isEnabled(thandler) ? Qt::Checked : Qt::Unchecked;

        } else if (role == Qt::ToolTipRole)
            if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){
                if (engine.isEnabled(thandler))
                    return tr("Disable trigger query handler.");
                else
                    return tr("Enable trigger query handler.");
            }

    } else if (idx.column() == (int) Column::GHandler) {
        if (auto *ghandler = dynamic_cast<GlobalQueryHandler*>(handler); ghandler){
            if (role == Qt::CheckStateRole)
                return engine.isEnabled(ghandler) ? Qt::Checked : Qt::Unchecked;
            else if (role == Qt::ToolTipRole){
                if (engine.isEnabled(ghandler))
                    return tr("Disable global query handler.");
                else
                    return tr("Enable global query handler.");
            }
        }

    } else if (idx.column() == (int) Column::FHandler) {
        if (auto *fhandler = dynamic_cast<FallbackHandler*>(handler); fhandler){
            if (role == Qt::CheckStateRole)
                return engine.isEnabled(fhandler) ? Qt::Checked : Qt::Unchecked;
            else if (role == Qt::ToolTipRole){
                if (engine.isEnabled(fhandler))
                    return tr("Disable fallback query handler.");
                else
                    return tr("Enable fallback query handler.");
            }
        }

    } else if (idx.column() == (int) Column::Fuzzy) {
        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler && thandler->supportsFuzzyMatching()){
            if (role == Qt::CheckStateRole)
                return engine.fuzzy(thandler) ? Qt::Checked : Qt::Unchecked;
            else if (role == Qt::ToolTipRole){
                if (engine.fuzzy(thandler))
                    return tr("Disable fuzzy matching.");
                else
                    return tr("Enable fuzzy matching.");
            }
        }

    } else if (idx.column() == (int) Column::Description) {
        if (role == Qt::DisplayRole)
            return handler->description();
    }
    return {};
}

bool HandlerModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    auto &handler = handlers[idx.row()];

    if (idx.column() == (int) Column::Trigger) {
        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){
            if (role == Qt::EditRole) {
                if (auto err = engine.setTrigger(thandler, value.toString()); !err.isNull())
                    QMessageBox::warning(nullptr, qApp->applicationName(), err);
                emit dataChanged(index(idx.row(), idx.column()+1), idx, {Qt::DisplayRole});
                return true;
            }
        }

    } else if (idx.column() == (int)Column::THandler) {
        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){
            if (role == Qt::CheckStateRole) {
                if (auto err = engine.setEnabled(thandler, value == Qt::Checked); !err.isNull())
                    QMessageBox::warning(nullptr, qApp->applicationName(), err);
                emit dataChanged(index(idx.row(), idx.column()-1), idx, {Qt::DisplayRole});
                return true;
            }
        }

    } else if (idx.column() == (int) Column::GHandler) {
        if (auto *ghandler = dynamic_cast<GlobalQueryHandler*>(handler); ghandler){
            if (role == Qt::CheckStateRole) {
                engine.setEnabled(ghandler, value == Qt::Checked);
                return true;
            }
        }

    } else if (idx.column() == (int) Column::FHandler) {
        if (auto *fhandler = dynamic_cast<FallbackHandler*>(handler); fhandler){
            if (role == Qt::CheckStateRole) {
                engine.setEnabled(fhandler, value == Qt::Checked);
                return true;
            }
        }

    } else if (idx.column() == (int) Column::Fuzzy) {
        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){
            if (role == Qt::CheckStateRole) {
                engine.setFuzzy(thandler, value == Qt::Checked);
                return true;
            }
        }
    }

    return false;
}

QVariant HandlerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
        switch ((Column) section) {
        case Column::Name:        return tr("Extension");
        case Column::Trigger:     return tr("Trigger");
        case Column::THandler:    return tr("T", "short Trigger");
        case Column::GHandler:    return tr("G", "short Global");
        case Column::FHandler:    return tr("F", "short Fallback");
        case Column::Fuzzy:       return tr("Fz", "short Fuzzy");
        case Column::Description: return tr("Description");
        }
    else if (role == Qt::ToolTipRole)
        switch ((Column) section) {
        case Column::Name:        return headerData(section, orientation, Qt::DisplayRole);
        case Column::Trigger:     return tr("The trigger of the handler. Spaces are visualized by •.");
        case Column::THandler:    return tr("Enabled trigger query handlers.");
        case Column::GHandler:    return tr("Enabled global query handlers.");
        case Column::FHandler:    return tr("Enabled fallback query handlers.");
        case Column::Fuzzy:       return tr("Fuzzy matching.");
        case Column::Description: return headerData(section, orientation, Qt::DisplayRole);
        }
    return {};
}

Qt::ItemFlags HandlerModel::flags(const QModelIndex &idx) const
{
    switch ((Column) idx.column()) {
    case Column::Name:
    case Column::Description:
        return Qt::NoItemFlags;//Qt::ItemIsEnabled;
    case Column::Trigger:
        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handlers[idx.row()]); thandler && thandler->allowTriggerRemap())
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
        else
            return Qt::NoItemFlags;
    case Column::THandler:
        return dynamic_cast<TriggerQueryHandler*>(handlers[idx.row()]) ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable : Qt::NoItemFlags;
    case Column::GHandler:
        return dynamic_cast<GlobalQueryHandler*>(handlers[idx.row()]) ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable : Qt::NoItemFlags;
    case Column::FHandler:
        return dynamic_cast<FallbackHandler*>(handlers[idx.row()]) ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable : Qt::NoItemFlags;
    case Column::Fuzzy:
        if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handlers[idx.row()]); thandler && thandler->supportsFuzzyMatching())
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        else
            return Qt::NoItemFlags;
    }
    return {};
}

void HandlerModel::addHandler(Extension *handler)
{
    auto it = lower_bound(begin(handlers), end(handlers), handler,
                          [](const auto &a, const auto &b) { return a->name() < b->name(); });

    if (it == handlers.end() || (*it)->id() != handler->id()){  // if not registered
        auto i = std::distance(begin(handlers), it);
        beginInsertRows(QModelIndex(), i, i);
        handlers.insert(it, handler);
        endInsertRows();
    }
}

void HandlerModel::removeHandler(Extension *handler)
{
    auto it = std::find_if(begin(handlers), end(handlers),
                           [handler](const auto &h) { return h->id() ==  handler->id(); });
    if (it != handlers.end()){  // exists at all
        auto i = std::distance(begin(handlers), it);
        beginRemoveRows(QModelIndex(), i, i);
        handlers.erase(it);
        endRemoveRows();
    }
}
