// Copyright (c) 2022-2023 Manuel Schneider

#include "QtCore/qiodevice.h"
#include "QtCore/qmimedata.h"
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extensionwatcher.h"
#include "albert/util/iconprovider.h"
#include "fallbackwidget.h"
#include "queryengine.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <set>
using namespace albert;
using namespace std;

enum class Column {
    Name,
//    Short,
    Description,
};

class FallbackModel : public QAbstractTableModel, ExtensionWatcher<FallbackHandler>
{
public:

    IconProvider ip;
    set<FallbackHandler*> fallback_handlers;

    vector<shared_ptr<Item>> fallbacks;

    map<QString, int> fallback_order;
    mutable map<QString, QIcon> iconCache;

//    QueryEngine &engine;
//    albert::ExtensionRegistry &registry;

    explicit FallbackModel(QueryEngine &qe, albert::ExtensionRegistry &registry, QObject *parent):
        QAbstractTableModel(parent), ExtensionWatcher<FallbackHandler>(&registry) //, engine(qe), registry(er)
    {
        for (auto &[id, handler]: registry.extensions<FallbackHandler>())
            fallback_handlers.emplace(handler);

        updateFallbacks();
    }

    void updateFallbacks()
    {
        beginResetModel();
        fallbacks.clear();
        for (auto *h : fallback_handlers)
            for (auto &fb : h->fallbacks("…"))
                fallbacks.emplace_back(fb);
        endResetModel();
    }

    void onAdd(FallbackHandler *handler) override
    {
        fallback_handlers.emplace(handler);
        updateFallbacks();
    }

    void onRem(FallbackHandler *handler) override
    {
        fallback_handlers.erase(handler);
        updateFallbacks();
    }

    int rowCount(const QModelIndex &) const override { return (int)fallbacks.size(); }

    int columnCount(const QModelIndex &) const override { return 2; }

    QVariant data(const QModelIndex &idx, int role) const override
    {
        auto &fb = fallbacks[idx.row()];

        if (idx.column() == (int) Column::Name) {
            if (role == Qt::DisplayRole)
                return fb->text();
            else if (role == Qt::DecorationRole){

                // Resizing request thounsands of repaints. Creating an icon for
                // ever paint event is to expensive. Therefor maintain an icon cache

                try {
                    return iconCache.at(fb->id());
                } catch (const out_of_range&) {
                    QSize s(32, 32);
                    return iconCache.emplace(fb->id(), QIcon(ip.getPixmap(fb->iconUrls(), &s, s))).second;
                }
            }
            else if (role == Qt::ToolTipRole)
                return fb->id();
        } else if (idx.column() == (int) Column::Description) {
            if (role == Qt::DisplayRole)
                return fb->subtext();
        }
        return {};
    }

    bool setData(const QModelIndex &idx, const QVariant &value, int role) override
    {
//        auto &handler = handlers[idx.row()];

//        if (idx.column() == (int) Column::Trigger) {
//            if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){
//                if (role == Qt::EditRole) {
//                    if (auto err = engine.setTrigger(thandler, value.toString()); !err.isNull())
//                        QMessageBox::warning(nullptr, qApp->applicationName(), err);
//                    emit dataChanged(index(idx.row(), idx.column()+1), idx, {Qt::DisplayRole});
//                    return true;
//                }
//            }

//        } else if (idx.column() == (int)Column::THandler) {
//            if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){
//                if (role == Qt::CheckStateRole) {
//                    if (auto err = engine.setEnabled(thandler, value == Qt::Checked); !err.isNull())
//                        QMessageBox::warning(nullptr, qApp->applicationName(), err);
//                    emit dataChanged(index(idx.row(), idx.column()-1), idx, {Qt::DisplayRole});
//                    return true;
//                }
//            }

//        } else if (idx.column() == (int) Column::GHandler) {
//            if (auto *ghandler = dynamic_cast<GlobalQueryHandler*>(handler); ghandler){
//                if (role == Qt::CheckStateRole) {
//                    engine.setEnabled(ghandler, value == Qt::Checked);
//                    return true;
//                }
//            }

//        } else if (idx.column() == (int) Column::FHandler) {
//            if (auto *fhandler = dynamic_cast<FallbackHandler*>(handler); fhandler){
//                if (role == Qt::CheckStateRole) {
//                    engine.setEnabled(fhandler, value == Qt::Checked);
//                    return true;
//                }
//            }

//        } else if (idx.column() == (int) Column::Fuzzy) {
//            if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handler); thandler){
//                if (role == Qt::CheckStateRole) {
//                    engine.setFuzzy(thandler, value == Qt::Checked);
//                    return true;
//                }
//            }
//        }

        return false;
    }

    QVariant headerData(int section, Qt::Orientation, int role) const override
    {
        if (role == Qt::DisplayRole)
            switch ((Column) section) {
            case Column::Name:        return QStringLiteral("Handler");
//            case Column::Trigger:     return QStringLiteral("Trigger");
            case Column::Description: return QStringLiteral("Description");
            }
        return {};
    }


    Qt::DropActions supportedDropActions() const override
    {
        return Qt::MoveAction;
    }



    bool dropMimeData(const QMimeData *data, Qt::DropAction /*action*/,
                      int dstRow, int /*column*/, const QModelIndex &/*parent*/) override
    {
        QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&encoded, QIODevice::ReadOnly);
        int srcRow = 0;
        if (!stream.atEnd())
            stream >> srcRow;
        moveRows(QModelIndex(), srcRow, 1, QModelIndex(), dstRow);
        return false;
    }

    bool moveRows(const QModelIndex &srcParent, int srcRow, int cnt, const QModelIndex &dstParent, int dstRow) override
    {
        if (srcRow < 0 || cnt < 1 || dstRow < 0 ||
            static_cast<int>(fallbacks.size()) < srcRow + cnt - 1 ||
            static_cast<int>(fallbacks.size()) < dstRow ||
            ( srcRow <= dstRow && dstRow < srcRow + cnt) ) // If its inside the source do nothing
            return false;

        beginMoveRows(srcParent, srcRow, srcRow + cnt - 1, dstParent, dstRow);

        if (srcRow < dstRow)
            rotate(fallbacks.begin() + srcRow,
                   fallbacks.begin() + srcRow + cnt,
                   fallbacks.begin() + dstRow);
        else
            rotate(fallbacks.begin() + dstRow,
                   fallbacks.begin() + srcRow,
                   fallbacks.begin() + srcRow + cnt);

        endMoveRows();
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex &idx) const override
    {

        if (idx.isValid())
            return QAbstractTableModel::flags(idx) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
        else
            return QAbstractTableModel::flags(idx) | Qt::ItemIsDropEnabled;


        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

        switch ((Column) idx.column()) {
        case Column::Name:
        case Column::Description:
            return Qt::NoItemFlags;//Qt::ItemIsEnabled;
//        case Column::Trigger:
//            if (auto *thandler = dynamic_cast<TriggerQueryHandler*>(handlers[idx.row()]); thandler && thandler->allowTriggerRemap())
//                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
//            else
//                return Qt::NoItemFlags;
        }
        return {};
    }
};


FallbackWidget::FallbackWidget(QueryEngine &qe, ExtensionRegistry &er, QWidget *parent)
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
    setModel(model = new FallbackModel(qe, er, this)); // Takes ownership
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QTableView::DoubleClicked|QTableView::SelectedClicked|QTableView::EditKeyPressed);

    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ContiguousSelection);

}

FallbackWidget::~FallbackWidget() = default;
