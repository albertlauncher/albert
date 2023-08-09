// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extension/frontend/itemroles.h"
#include "albert/extension/queryhandler/item.h"
#include "albert/extension/queryhandler/rankitem.h"
#include "albert/logging.h"
#include "itemsmodel.h"
#include "usagedatabase.h"
#include "query.h"
#include "qmlrolenames.h"
#include <QStringListModel>
#include <QTimer>
using namespace albert;
using namespace std;


ItemsModel::ItemsModel(QObject *parent) : QAbstractListModel(parent) {}

int ItemsModel::rowCount(const QModelIndex &) const { return (int)items.size(); }

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const auto &[extension, item] = items[index.row()];

        switch (role) {
            case (int)ItemRoles::TextRole:{
                QString text = item->text();
                text.replace('\n', ' ');
                return text;
            }
            case (int)ItemRoles::SubTextRole:{
                QString text = item->subtext();
                text.replace('\n', ' ');
                return text;
            }
            case Qt::ToolTipRole: return QString("%1\n%2").arg(item->text(), item->subtext());
            case (int)ItemRoles::InputActionRole: return item->inputActionText();
            case (int)ItemRoles::IconUrlsRole: return item->iconUrls();
        }
    }
    return {};
}

QHash<int, QByteArray> ItemsModel::roleNames() const { return albert::QmlRoleNames; }

void ItemsModel::add(Extension *extension, shared_ptr<Item> &&item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(extension, ::move(item));
    endInsertRows();
}

void ItemsModel::add(Extension *extension, vector<shared_ptr<Item>> &&itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    items.reserve(items.size()+itemvec.size());
    for (auto &&item : itemvec)
        items.emplace_back(extension, ::move(item));
    endInsertRows();
}

void ItemsModel::add(Extension *extension, const shared_ptr<Item> &item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(extension, item);
    endInsertRows();
}

void ItemsModel::add(Extension *extension, const vector<shared_ptr<Item>> &itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    items.reserve(items.size()+itemvec.size());
    for (auto &item : itemvec)
        items.emplace_back(extension, item);
    endInsertRows();
}

void ItemsModel::add(vector<pair<Extension*,RankItem>>::iterator begin,
                     vector<pair<Extension*,RankItem>>::iterator end)
{
    if (begin == end)
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);
    items.reserve(items.size()+(size_t)(end-begin));
    for (auto it = begin; it != end; ++it)
        items.emplace_back(it->first, ::move(it->second.item));
    endInsertRows();
}

QAbstractListModel *ItemsModel::buildActionsModel(uint i) const
{
    QStringList l;
    for (const auto &a : items[i].second->actions())
        l << a.text;
    return new QStringListModel(l);
}

void ItemsModel::activate(QueryBase *q, uint i, uint a)
{
    if (i<items.size()){
        auto &[extension, item] = items[i];
        auto actions = item->actions();
        if (a<actions.size()){
            // sane context arg. it is intended to be executed later out of context.
            // QTimer::singleShot… dont. query has to stay alive as indicator for pluginregistry
            UsageHistory::addActivation(q->string(), extension->id(), item->id(), actions[a].id);
            actions[a].function(); // afterwards because query is dea

        }
        else
            WARN << "Activated action index is invalid.";
    }
    else
        WARN << "Activated item index is invalid.";
}
