// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/frontend.h"
#include "albert/extensions/globalqueryhandler.h"
#include "albert/logging.h"
#include "itemsmodel.h"
#include "queryengine.h"
using namespace std;
using namespace albert;

IconProvider ItemsModel::icon_provider;

int ItemsModel::rowCount(const QModelIndex &parent) const
{
    return (int)items.size();
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const shared_ptr<Item> &item = items[index.row()].second;

        switch (role) {
            case (int)ItemRoles::TextRole: return item->text();
            case (int)ItemRoles::SubTextRole: return item->subtext();
            case Qt::ToolTipRole: return QString("%1\n%2").arg(item->text(), item->subtext());
            case (int)ItemRoles::InputActionRole: return item->inputActionText();
            case (int)ItemRoles::IconUrlsRole: return item->iconUrls();
            case (int)ItemRoles::IconPathRole: qFatal("ItemsModel::data ItemRoles::IconPathRole not implemented");
            case (int)ItemRoles::IconRole:
            {
                auto get_icon_for_urls = [](const QStringList &urls)
                {
                    static map<QString, QIcon> icon_cache;
                    for (const QString &url : urls)
                        try {
                            return icon_cache.at(url);
                        } catch (const out_of_range &) {
                            if (auto && icon = icon_provider.getIcon(url); icon.isNull())
                                continue;
                            else
                                return icon_cache.emplace(url, icon).first->second;
                        }
                    return QIcon();
                };

                static map<QString, QIcon> item_icon_cache;
                try {
                    return item_icon_cache.at(item->id());
                } catch (const out_of_range &) {
                    QIcon icon = get_icon_for_urls(item->iconUrls());
                    return item_icon_cache.emplace(item->id(), icon.isNull() ? QIcon(":unknown") : icon).first->second;
                }
            }
        }
    }
    return {};
}

QHash<int, QByteArray> ItemsModel::roleNames() const
{
    return {
        {(int)ItemRoles::TextRole, "itemTextRole"},
        {(int)ItemRoles::SubTextRole, "itemSubTextRole"},
        {(int)ItemRoles::IconPathRole, "itemIconRole"},
        {(int)ItemRoles::InputActionRole, "itemInputActionRole"}
    };
}

void ItemsModel::add(albert::QueryHandler *handler, shared_ptr<Item> &&item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(handler, ::move(item));
    endInsertRows();
}

void ItemsModel::add(albert::QueryHandler *handler, vector<std::shared_ptr<Item>> &&itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    items.reserve(items.size()+itemvec.size());
    for (auto &&item : itemvec)
        items.emplace_back(handler, ::move(item));
    endInsertRows();
}

void ItemsModel::add(albert::QueryHandler *handler, const shared_ptr<albert::Item> &item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(handler, item);
    endInsertRows();
}

void ItemsModel::add(albert::QueryHandler *handler, const vector<std::shared_ptr<albert::Item>> &itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    items.reserve(items.size()+itemvec.size());
    for (auto &item : itemvec)
        items.emplace_back(handler, item);
    endInsertRows();
}


void ItemsModel::add(vector<pair<QueryHandler*,RankItem>>::iterator begin,
                     vector<pair<QueryHandler*,RankItem>>::iterator end)
{
    if (begin == end)
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);
    items.reserve(items.size()+(size_t)(end-begin));
    for (auto it = begin; it != end; ++it)
        items.emplace_back(it->first, ::move(it->second.item));
    endInsertRows();
}
