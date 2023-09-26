// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extensionwatcher.h"
#include "handlerwidget.h"
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

class HandlerModel : public QAbstractTableModel,
                     public ExtensionWatcher<TriggerQueryHandler>,
                     public ExtensionWatcher<GlobalQueryHandler>,
                     public ExtensionWatcher<FallbackHandler>
{
public:
    vector<Extension*> handlers;
    QueryEngine &engine;
    albert::ExtensionRegistry &registry;

    explicit HandlerModel(QueryEngine &qe, albert::ExtensionRegistry &er, QObject *parent):
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

    void onAdd(TriggerQueryHandler *handler) override { addHandler(handler); }
    void onAdd(GlobalQueryHandler *handler) override { addHandler(handler); }
    void onAdd(FallbackHandler *handler) override { addHandler(handler); }
    void onRem(TriggerQueryHandler *handler) override { removeHandler(handler); }
    void onRem(GlobalQueryHandler *handler) override { removeHandler(handler); }
    void onRem(FallbackHandler *handler) override { removeHandler(handler); }

    void addHandler(Extension *handler)
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

    void removeHandler(Extension *handler)
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

    int rowCount(const QModelIndex &) const override { return (int)handlers.size(); }

    int columnCount(const QModelIndex &) const override { return 7; }

    QVariant data(const QModelIndex &idx, int role) const override
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
                        return "This extension does not allow trigger remapping.";

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
                if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler)
                    return QString("%1 trigger query handler.").arg(engine.isEnabled(thandler) ? "Disable" : "Enable");

        } else if (idx.column() == (int) Column::GHandler) {
            if (auto *ghandler = dynamic_cast<GlobalQueryHandler*>(handler); ghandler){
                if (role == Qt::CheckStateRole)
                    return engine.isEnabled(ghandler) ? Qt::Checked : Qt::Unchecked;
                else if (role == Qt::ToolTipRole)
                    return QString("%1 global query handler.").arg(engine.isEnabled(ghandler) ? "Disable" : "Enable");
            }

        } else if (idx.column() == (int) Column::FHandler) {
            if (auto *fhandler = dynamic_cast<FallbackHandler*>(handler); fhandler){
                if (role == Qt::CheckStateRole)
                    return engine.isEnabled(fhandler) ? Qt::Checked : Qt::Unchecked;
                else if (role == Qt::ToolTipRole)
                    return QString("%1 fallback query handler.").arg(engine.isEnabled(fhandler) ? "Disable" : "Enable");
            }

        } else if (idx.column() == (int) Column::Fuzzy) {
            if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler && thandler->supportsFuzzyMatching()){
                if (role == Qt::CheckStateRole)
                    return engine.fuzzy(thandler) ? Qt::Checked : Qt::Unchecked;
                else if (role == Qt::ToolTipRole)
                    return QString("%1 fuzzy string matching.").arg(engine.fuzzy(thandler) ? "Disable" : "Enable");
            }

        } else if (idx.column() == (int) Column::Description) {
            if (role == Qt::DisplayRole)
                return handler->description();
        }
        return {};
    }

    bool setData(const QModelIndex &idx, const QVariant &value, int role) override
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

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role == Qt::DisplayRole)
            switch ((Column) section) {
            case Column::Name:        return QStringLiteral("Handler");
            case Column::Trigger:     return QStringLiteral("Trigger");
            case Column::THandler:    return QStringLiteral("TH");
            case Column::GHandler:    return QStringLiteral("GH");
            case Column::FHandler:    return QStringLiteral("FH");
            case Column::Fuzzy:       return QStringLiteral("Fz");
            case Column::Description: return QStringLiteral("Description");
            }
        else if (role == Qt::ToolTipRole)
            switch ((Column) section) {
            case Column::Name:        return headerData(section, orientation, Qt::DisplayRole);
            case Column::Trigger:     return headerData(section, orientation, Qt::DisplayRole);
            case Column::THandler:    return QStringLiteral("Enabled trigger query handlers.");
            case Column::GHandler:    return QStringLiteral("Enabled global query handlers.");
            case Column::FHandler:    return QStringLiteral("Enabled fallback query handlers.");
            case Column::Fuzzy:       return QStringLiteral("Fuzzy string matching.");
            case Column::Description: return headerData(section, orientation, Qt::DisplayRole);
            }
        return {};
    }

    Qt::ItemFlags flags(const QModelIndex &idx) const override
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
};


HandlerWidget::HandlerWidget(QueryEngine &qe, ExtensionRegistry &er, QWidget *parent)
    : QTableView(parent)
{
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->hide();

    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionsClickable(false);

    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);
    setAlternatingRowColors(true);
    QAbstractTableModel *model;
    setModel(model = new HandlerModel(qe, er, this)); // Takes ownership
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QTableView::DoubleClicked|QTableView::SelectedClicked|QTableView::EditKeyPressed);


    // Select first selectable item
    // CAUTION: returns!
    for (int row = 0; row < model->rowCount(); ++row)
        for (int col = 0; col < model->columnCount(); ++col)
            if (auto index = model->index(row, col); index.flags() & Qt::ItemIsEnabled) {
                selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
                return;
            }
}

HandlerWidget::~HandlerWidget() = default;
