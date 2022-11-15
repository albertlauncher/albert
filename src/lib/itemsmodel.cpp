// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/frontend.h"
#include "albert/extensions/queryhandler.h"
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
        const shared_ptr<Item> &item = items[index.row()];

        switch (static_cast<ItemRoles>(role)) {
            case ItemRoles::TextRole: return item->text();
            case ItemRoles::SubTextRole: return item->subtext();
            case ItemRoles::InputActionRole: return item->inputActionText();
            case ItemRoles::IconUrlsRole: return item->iconUrls();
            case ItemRoles::IconPathRole: qFatal("ItemsModel::data ItemRoles::IconPathRole not implemented");
            case ItemRoles::IconRole:
            {
                auto get_icon_for_urls = [](const QStringList &urls)
                {
                    static map<QString, QIcon> icon_cache;
                    for (const QString &url : urls)
                        try {
                            return icon_cache.at(url);
                        } catch (out_of_range &) {
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
                } catch (out_of_range &) {
                    QIcon &&icon = get_icon_for_urls(item->iconUrls());
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

void ItemsModel::add(shared_ptr<Item> &&item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(::move(item));
    endInsertRows();
}

void ItemsModel::add(vector<std::shared_ptr<Item>> &&itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    if (!items.empty())
        items = ::move(itemvec);
    else
        items.insert(end(items),
                     make_move_iterator(begin(itemvec)),
                     make_move_iterator(end(itemvec)));
    endInsertRows();
}

void ItemsModel::add(const shared_ptr<albert::Item> &item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(item);
    endInsertRows();
}

void ItemsModel::add(const vector<std::shared_ptr<albert::Item>> &itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    items.insert(end(items), begin(itemvec), end(itemvec));
    endInsertRows();

}
