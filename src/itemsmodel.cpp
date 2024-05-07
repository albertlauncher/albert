// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/extension.h"
#include "albert/frontend.h"
#include "albert/logging.h"
#include "albert/query/item.h"
#include "albert/query/query.h"
#include "albert/query/rankitem.h"
#include "itemsmodel.h"
#include "usagedatabase.h"
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
            case (int)ItemRoles::TextRole:
            {
                QString text = item->text();
                text.replace('\n', ' ');
                return text;
            }
            case (int)ItemRoles::SubTextRole:
            {
                QString text = item->subtext();
                text.replace('\n', ' ');
                return text;
            }
            case Qt::ToolTipRole:
                return QString("%1\n%2").arg(item->text(), item->subtext());

            case (int)ItemRoles::InputActionRole:
                return item->inputActionText();

            case (int)ItemRoles::IconUrlsRole:
                return item->iconUrls();

            case (int)ItemRoles::ActionsListRole:
            {
                if (auto it = actionsCache.find(make_pair(extension, item.get()));
                    it != actionsCache.end())
                    return it->second;

                QStringList l;
                for (const auto &a : item->actions())
                    l << a.text;

                actionsCache.emplace(make_pair(extension, item.get()), l);

                return l;
            }
        }
    }
    return {};
}

QHash<int, QByteArray> ItemsModel::roleNames() const
{
    static QHash<int, QByteArray> qml_role_names = {
        {(int)ItemRoles::TextRole, "itemText"},
        {(int)ItemRoles::SubTextRole, "itemSubText"},
        {(int)ItemRoles::InputActionRole, "itemInputAction"},
        {(int)ItemRoles::IconUrlsRole, "itemIconUrls"},
        {(int)ItemRoles::ActionsListRole, "itemActionsList"}
    };
    return qml_role_names;
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

void ItemsModel::add(vector<pair<Extension*, shared_ptr<Item>>>::iterator begin,
                     vector<pair<Extension*, shared_ptr<Item>>>::iterator end)
{
    if (begin == end)
        return;

    items.reserve(items.size()+(size_t)(end-begin));

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);

    items.insert(items.end(), make_move_iterator(begin), make_move_iterator(end));

    endInsertRows();

}

void ItemsModel::add(vector<pair<Extension*,RankItem>>::iterator begin,
                     vector<pair<Extension*,RankItem>>::iterator end)
{
    if (begin == end)
        return;

    items.reserve(items.size()+(size_t)(end-begin));

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);

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

void ItemsModel::activate(Query *q, uint i, uint a)
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
