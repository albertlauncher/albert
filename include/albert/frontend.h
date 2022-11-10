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
class Item;

struct ALBERT_EXPORT Query : public QObject
{
    ~Query() override = default;
    virtual const QString &synopsis() const = 0;  /// The trigger of this query.
    virtual const QString &trigger() const = 0;  /// The trigger of this query.
    virtual const QString &string() const = 0;  /// Query string _excluding_ the trigger.
    virtual bool isFinished() const = 0;  /// Asynchronous query processing done.
    virtual void run() = 0;  /// Asynchronous query processing done.
    virtual void cancel() = 0;  /// Call this if you dont need the query anymore
    virtual const std::vector<std::shared_ptr<Item>> &results() const = 0;
    virtual void activateResult(uint item, uint action) = 0;
Q_OBJECT signals:
    void resultsChanged();
    void finished();
};

/// The interface for albert frontends
struct ALBERT_EXPORT Frontend : virtual public Extension
{
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual QString input() const = 0;
    virtual void setInput(const QString&) = 0;
    virtual QWidget *createSettingsWidget() = 0;
    void setEngine(QueryEngine*);

protected:
    std::unique_ptr<Query> query(const QString &query) const;

private:
    QueryEngine *query_engine;
};

/// Convention on the item roles passed around (ResultsModel, Frontends)
enum class ALBERT_EXPORT ItemRoles {
    TextRole = Qt::DisplayRole,
    IconRole = Qt::DecorationRole,
    SubTextRole = Qt::ToolTipRole,
    InputActionRole = Qt::UserRole, // Note this is used as int in QML
};

struct ALBERT_EXPORT ItemModel : public QAbstractListModel
{
    explicit ItemModel(Query *);
    inline int rowCount(const QModelIndex &) const override;
    QHash<int,QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
private:
    int row_count = 0;
    Query *query_;
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

