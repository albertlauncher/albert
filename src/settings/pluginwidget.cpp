// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/extensions/pluginprovider.h"
#include "pluginregistry.h"
#include "pluginwidget.h"
#include <QAbstractTableModel>
#include <QFormLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPlainTextEdit>
#include <algorithm>
#include <map>
#include <set>
using namespace std;
using namespace albert;

enum class Column
{
    Enabled,
    State,
    Name,
    Version,
    Description,
};

class  PluginModel: public QAbstractTableModel
{
public:
    explicit PluginModel(PluginRegistry &plugin_registry)
        : plugin_registry_(plugin_registry)
    {
        connect(&plugin_registry, &PluginRegistry::pluginsChanged,
                this, &PluginModel::updatePlugins);
        updatePlugins();
    }

    void onActivate(const QModelIndex &index) { plugins_.at(index.row())->makeInfoWidget()->show(); }

private:

    QIcon getCachedIcon(const QString &url) const
    {
        try {
            return icon_cache.at(url);
        } catch (const std::out_of_range &e) {
            return icon_cache.emplace(url, url).first->second;
        }
    }

    int rowCount(const QModelIndex&) const override { return static_cast<int>(plugins_.size()); }

    int columnCount(const QModelIndex&) const override { return 5; }

    QVariant data(const QModelIndex &idx, int role) const override
    {
        if (!idx.isValid()
            || idx.row() < 0 || rowCount(idx.parent()) <= idx.row()
            || idx.column() < 0 || columnCount(idx.parent()) <= idx.column())
            return {};

        const auto *p = plugins_[idx.row()];

        switch (static_cast<Column>(idx.column())) {
        case Column::Enabled:
            if (role == Qt::CheckStateRole && p->metaData().user)
                return (plugin_registry_.isEnabled(p->metaData().id)) ? Qt::Checked : Qt::Unchecked;
            break;

        case Column::State:
            if (role == Qt::DecorationRole)
                switch (p->state()) {
                case PluginState::Loaded:
                    return getCachedIcon(":plugin_loaded");
                case PluginState::Unloaded:
                    if (p->stateInfo().isEmpty())
                        return getCachedIcon(":plugin_unloaded");
                    else
                        return getCachedIcon(":plugin_error");
                case PluginState::Invalid:
                    break;
                }
            else if (role == Qt::ToolTipRole && !p->stateInfo().isEmpty())
                return QString("<span style=\"color:#ff0000;\">%1</span>").arg(p->stateInfo());
            break;

        case Column::Name:
            if (role == Qt::DisplayRole)
                return p->metaData().name;
            else if (role == Qt::DecorationRole)
                return getCachedIcon(p->iconUrl());
            break;

        case Column::Version:
            if (role == Qt::DisplayRole)
                return p->metaData().version;
            break;

        case Column::Description:
            if (role == Qt::DisplayRole)
                return p->metaData().description;
            break;
        }
        return {};
    }

    bool setData(const QModelIndex &idx, const QVariant &value, int role) override
    {
        if (idx.isValid()
            && static_cast<Column>(idx.column()) == Column::Enabled
            && role == Qt::CheckStateRole){
            try {
                const auto *p = plugins_[idx.row()];
                plugin_registry_.enable(p->metaData().id, value == Qt::Checked);
                emit dataChanged(index(idx.row(), (int)Column::Enabled),
                                 index(idx.row(), (int)Column::State));
                return true;
            } catch (std::out_of_range &e){}
        }
        return false;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            switch (static_cast<Column>(section)) {
            case Column::Enabled:
            case Column::State:
                break;
            case Column::Name:
                return "Name";
            case Column::Version:
                return "Version";
            case Column::Description:
                return "Description";
            }
        return {};
    }

    Qt::ItemFlags flags(const QModelIndex &idx) const override
    {
        if (!idx.isValid() || idx.row() < 0 || rowCount(idx.parent()) <= idx.row())
            return Qt::NoItemFlags;
        else if (plugins_[idx.row()]->metaData().user)
            return Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        else
            return Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled;
    }

    void updatePlugins()
    {
        beginResetModel();
        plugins_ = plugin_registry_.plugins();
        ::sort(plugins_.begin(), plugins_.end(),
               [](const auto &l, const auto &r){ return l->metaData().name < r->metaData().name; });
        endResetModel();
    }

    PluginRegistry &plugin_registry_;
    std::vector<const PluginLoader*> plugins_;
    mutable std::map<QString, QIcon> icon_cache;
};


PluginWidget::PluginWidget(PluginRegistry &plugin_registry) : model_(new PluginModel(plugin_registry))
{
    setModel(model_.get());

//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlternatingRowColors(true);
    setShowGrid(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setFrameShape(QFrame::NoFrame);

    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->hide();

    horizontalHeader()->setFrameShape(QFrame::NoFrame);
    horizontalHeader()->setSectionsClickable(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setStretchLastSection(true);

    connect(this, &QTableView::activated, model_.get(), &PluginModel::onActivate);

}

PluginWidget::~PluginWidget() = default;
