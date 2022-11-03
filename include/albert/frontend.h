// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include <QAbstractItemModel>
#include <QWidget>
#include <memory>
#include <functional>
class QueryEngine;

namespace albert
{
class Query;

/// Convention on the item roles passed around (ResultsModel, Frontends)
enum class ALBERT_EXPORT ItemRoles {
    TextRole = Qt::DisplayRole,
    IconRole = Qt::DecorationRole,
    SubTextRole = Qt::ToolTipRole,
    InputActionRole = Qt::UserRole, // Note this is used as int in QML
};


struct ALBERT_EXPORT ItemModel : public QAbstractListModel
{
    explicit ItemModel(albert::Query *);
    inline int rowCount(const QModelIndex &) const override;
    QHash<int,QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
private:
    int row_count = 0;
    albert::Query *query_;
};


/// The interface for albert frontends
class ALBERT_EXPORT Frontend : virtual public Extension
{
public:
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;

    virtual QString input() const = 0;
    virtual void setInput(const QString&) = 0;

    virtual QWidget *createSettingsWidget() = 0;

    void setEngine(QueryEngine*);

protected:
    std::unique_ptr<albert::Query> query(const QString &query) const;

private:
    QueryEngine *query_engine;
};
}









//                // TODO 0.18 has actions role, drop altactions return all actions
//            case ItemRoles::ActionRole:
//                return (0 < static_cast<int>(item->actions().size()))
//                       ? item->actions()[0]->text()
//                       : item->subtext();
//            case ItemRoles::AltActionRole: { // Actions list
//                QStringList actionTexts;
//                for (auto &action : item->actions())
//                    actionTexts.append(action->text());
//                return actionTexts;
//            }
//                    // TODO maybe 0.18 give fallbacks a dedicated model
//        case ItemRoles::FallbackRole:
//            return QString("Search '%1' using default fallback").arg(string_);


//
//
//
//bool ItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//
//    if (index.isValid()) {
//        const SharedItem &item = result_items[index.row()].item;
//
//        switch (static_cast<ItemRoles>(role)){
//            case ItemRoles::ActionRole:{
//                if (0U < item->actions().size()){
//                    item->actions()[0]->activate();
////                activations.emplace_back(std::get<QueryHandler*>(matches[index.row()]),item->id(),item->id());
////                activated_item = ; FIXME
//                }
//                return true;
//            }
//            case ItemRoles::AltActionRole:{
//                size_t actionValue = static_cast<size_t>(value.toInt());
//                if (actionValue < item->actions().size()) {
//                    item->actions()[actionValue]->activate();
////                activated_item = item->id();
//                }
//                return true;
//            }
//                // FIXME
////        case ItemRoles::FallbackRole:{
////            if (0U < fallbacks.size() && 0U < fallbacks[0].first->actions().size()) {
////                fallbacks_[0].first->actions()[0]->activate();
////                activated_item = fallbacks_[0].first->id();
////            }
////            break;
////        }
//            default:
//                return false;
//        }
//    }
//    return false;
//}

