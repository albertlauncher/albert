// Copyright (c) 2022-2023 Manuel Schneider

#include "queryengine.h"
#include "triggerwidget.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableView>
#include <QVBoxLayout>
using namespace albert;
using namespace std;

enum class Column {
    Name,
    Trigger,
    Description
};

class TriggerModel : public QAbstractTableModel, ExtensionWatcher<TriggerQueryHandler>
{
public:
    vector<TriggerQueryHandler *> handlers;
    QueryEngine &engine;

    explicit TriggerModel(QueryEngine &qe, ExtensionRegistry &er) :
            ExtensionWatcher<TriggerQueryHandler>(&er), engine(qe)
    {
        for (auto &[id, handler]: er.extensions<TriggerQueryHandler>())
            handlers.emplace_back(handler);

        ::sort(begin(handlers), end(handlers),
               [](const auto &a, const auto &b) { return a->name() < b->name(); });

    }

    void onAdd(TriggerQueryHandler *t) override
    {
        auto it = lower_bound(begin(handlers), end(handlers), t,
                              [](const auto &a, const auto &b) { return a->name() < b->name(); });
        auto i = std::distance(begin(handlers), it);
        beginInsertRows(QModelIndex(), i, i);
        handlers.insert(it, t);
        endInsertRows();
    }

    void onRem(TriggerQueryHandler *t) override
    {
        handlers.erase(remove(handlers.begin(), handlers.end(), t), handlers.end());
    }

    int rowCount(const QModelIndex &) const override
    {
        return (int) handlers.size();
    }

    int columnCount(const QModelIndex &) const override
    {
        return 3;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (index.column() == (int) Column::Name) {

            if (role == Qt::DisplayRole)
                return handlers[index.row()]->name();

            else if (role == Qt::ToolTipRole)
                return handlers[index.row()]->id();

        } else if (index.column() == (int) Column::Description) {

            if (role == Qt::DisplayRole)
                return handlers[index.row()]->description();

        } else if (index.column() == (int) Column::Trigger) {
            auto &handler = handlers[index.row()];

            if (role == Qt::DisplayRole) {
                return QString(engine.trigger(handler)).replace(" ", "•");

            } else if (role == Qt::EditRole) {
                return engine.trigger(handler);

            } else if (role == Qt::ToolTipRole) {
                if (!handler->allowTriggerRemap())
                    return "This extension does not allow trigger remapping.";

            } else if (role == Qt::CheckStateRole) {
                return engine.isEnabled(handler) ? Qt::Checked : Qt::Unchecked;

            } else if (role == Qt::FontRole && !handler->allowTriggerRemap()) {
                QFont f;
                f.setItalic(true);
                return f;

            } else if (role == Qt::ForegroundRole) {
                if (!engine.isEnabled(handler))
                    return QColor(Qt::gray);

            }
        }
        return {};
    }

    bool setData(const QModelIndex &idx, const QVariant &value, int role) override
    {
        if (idx.column() == (int) Column::Trigger) {
            if (role == Qt::EditRole) {
                if (engine.setTrigger(handlers[idx.row()], value.toString())){
                    emit dataChanged(index(0, (int) Column::Trigger),
                                     index((int) handlers.size(), (int) Column::Trigger),
                                     {Qt::DisplayRole});
                    return true;
                } else {
                    QMessageBox::warning(nullptr, qApp->applicationName(),
                                         QString("The tigger '%1' is already reserved.").arg(value.toString()));
                    return false;
                }
            }

            else if (role == Qt::CheckStateRole) {
                return engine.setEnabled(handlers[idx.row()], value == Qt::Checked);
                emit dataChanged(index(0, (int) Column::Trigger),
                                 index((int) handlers.size(), (int) Column::Trigger),
                                 {Qt::DisplayRole});
                return true;
            }
        }
        return false;
    }

    QVariant headerData(int section, Qt::Orientation, int role) const override
    {
        if (role == Qt::DisplayRole)
            switch ((Column) section) {
                case Column::Name: return "Extension";
                case Column::Trigger: return "Trigger";
                case Column::Description: return "Description";
            }
        return {};
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        switch ((Column) index.column()) {
            case Column::Name:
            case Column::Description:
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            case Column::Trigger:
                if (handlers[index.row()]->allowTriggerRemap())
                    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
                else
                    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            default:
                return {};
        }
    }
};

TriggerWidget::TriggerWidget(QueryEngine &qe, ExtensionRegistry &er)
    : model(new TriggerModel(qe, er))
{
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->hide();

    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionsClickable(false);

    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);
    setAlternatingRowColors(true);
    setModel(model.get());
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QTableView::DoubleClicked|QTableView::SelectedClicked|QTableView::EditKeyPressed);

    // Keep current item in clickable row
    connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex &current, const QModelIndex&){
        blockSignals(true);
        setCurrentIndex(model->index(current.row(), (int)Column::Trigger));
        blockSignals(false);
    });

    connect(this, &QTableView::activated, this, [this](const QModelIndex &index){
        edit(model->index(index.row(), (int)Column::Trigger));
    });
}

TriggerWidget::~TriggerWidget() = default;
