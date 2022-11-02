// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include "plugin.h"
#include "item.h"
#include <QAbstractItemModel>
#include <QString>
#include <QStringListModel>
#include <QWidget>

namespace albert
{

/// Convention on the item roles passed around (ResultsModel, Frontends)
enum class ALBERT_EXPORT ItemRoles {
    TextRole = Qt::DisplayRole,
    IconRole = Qt::DecorationRole,
    SubTextRole = Qt::ToolTipRole,
    InputActionRole = Qt::UserRole, // Note this is used as int in QML
};

static const std::map<const ItemRoles, const QByteArray> QmlRoleNames {
        { ItemRoles::TextRole, "itemTextRole"},
        { ItemRoles::SubTextRole, "itemSubTextRole"},
        { ItemRoles::IconRole, "itemIconRole"},
        { ItemRoles::InputActionRole, "itemInputActionRole"},
};

struct ALBERT_EXPORT ActionModel : public QStringListModel
{
    ActionModel(std::vector<Action> &&item_actions);
    const std::vector<Action> actions;
};

struct ALBERT_EXPORT ItemModel : public QAbstractListModel
{
    ItemModel(QObject *parent, const std::vector<std::shared_ptr<Item>> &items) : QAbstractListModel(parent), items(items) {}
    void updateView();

    inline int rowCount(const QModelIndex &) const override;
    QHash<int,QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    uint row_count = 0;
    const std::vector<std::shared_ptr<Item>> &items;
};

/// The interface for albert frontends
class ALBERT_EXPORT Frontend : virtual public Extension
{
public:
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible = true) = 0;

    virtual QString input() const = 0;
    virtual void setInput(const QString&) = 0;

    virtual QWidget *createSettingsWidget() = 0;
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

