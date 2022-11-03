// Copyright (c) 2022 Manuel Schneider

#include "albert/frontend.h"
#include "albert/item.h"
#include "albert/logging.h"
#include "albert/query.h"
#include "iconprovider.h"
#include "queryengine.h"
using namespace std;
using namespace albert;


static const std::map<const ItemRoles, const QByteArray> QmlRoleNames {
        { ItemRoles::TextRole, "itemTextRole"},
        { ItemRoles::SubTextRole, "itemSubTextRole"},
        { ItemRoles::IconRole, "itemIconRole"},
        { ItemRoles::InputActionRole, "itemInputActionRole"},
};

ItemModel::ItemModel(albert::Query *query) : query_(query)
{
    connect(query_, &Query::resultsChanged, this, [this](){
        beginInsertRows(QModelIndex(), row_count, static_cast<int>(query_->results().size()) - 1);
        row_count = static_cast<int>(query_->results().size());
        endInsertRows();
    });
}

int ItemModel::rowCount(const QModelIndex &) const
{
    return row_count;
}

QHash<int, QByteArray> ItemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    for (const auto role : {ItemRoles::TextRole,
                            ItemRoles::SubTextRole,
                            ItemRoles::IconRole,
                            ItemRoles::InputActionRole})
        roles[static_cast<int>(role)] = QmlRoleNames.at(role);
    return roles;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const shared_ptr<Item> &item = query_->results()[index.row()];

        switch (static_cast<ItemRoles>(role)) {
            case ItemRoles::TextRole: return item->text();
            case ItemRoles::SubTextRole: return item->subtext();
            case ItemRoles::InputActionRole: return item->inputActionText();
            case ItemRoles::IconRole:
            {
                auto get_icon_for_urls = [](const QStringList &urls)
                {
                    static IconProvider icon_provider;
                    static std::map<QString, QIcon> icon_cache;
                    for (const QString &url : urls)
                        try {
                            return icon_cache.at(url);
                        } catch (std::out_of_range &) {
                            if (auto && icon = icon_provider.getIcon(url); icon.isNull())
                                continue;
                            else
                                return icon_cache.emplace(url, icon).first->second;
                        }
                    return QIcon();
                };

                static std::map<QString, QIcon> item_icon_cache;
                try {
                    return item_icon_cache.at(item->id());
                } catch (std::out_of_range &) {
                    QIcon &&icon = get_icon_for_urls(item->iconUrls());
                    return item_icon_cache.emplace(item->id(), icon.isNull() ? QIcon(":unknown") : icon).first->second;
                }
            }
        }
    }
    return {};
}


std::unique_ptr<albert::Query> albert::Frontend::query(const QString &query) const
{
    return query_engine->query(query);
}

void Frontend::setEngine(QueryEngine *engine)
{
    query_engine = engine;
}
