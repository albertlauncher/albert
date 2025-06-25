// Copyright (c) 2022-2025 Manuel Schneider

#include "messagebox.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginregistry.h"
#include "pluginsmodel.h"
#include "pluginssortproxymodel.h"
#include "pluginswidget.h"
#include "pluginwidget.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QScrollArea>
using enum Plugin::State;
using enum albert::PluginMetadata::LoadType;
using namespace albert;
using namespace std;

PluginsWidget::PluginsWidget(PluginRegistry &plugin_registry):
    plugin_registry_(plugin_registry),
    model_(new PluginsModel(plugin_registry, this)),
    proxy_model_(new PluginsSortProxyModel(this))
{
    // Plugins list

    plugins_list_view_ = new QListView(this);
    plugins_list_view_->setModel(proxy_model_);
    proxy_model_->setSourceModel(model_);
    proxy_model_->setDynamicSortFilter(true);
    proxy_model_->sort(0);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    // Some styles on linux have bigger icons than rows
    auto rh = plugins_list_view_->sizeHintForRow(0);  // this requires a model
    plugins_list_view_->setIconSize(QSize(rh, rh));
#endif

    plugins_list_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plugins_list_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    plugins_list_view_->setProperty("showDropIndicator", QVariant(false));
    plugins_list_view_->setUniformItemSizes(true);

    updatePluginListWidth();
    connect(proxy_model_, &PluginsModel::modelReset,
            this, &PluginsWidget::updatePluginListWidth);

    plugins_list_view_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plugins_list_view_, &QListView::customContextMenuRequested,
            this, &PluginsWidget::showContextMenu);


    // Plugin config widget area

    config_widget_scroll_area_ = new QScrollArea(this);
    config_widget_scroll_area_->setFrameShape(QFrame::StyledPanel);
    config_widget_scroll_area_->setFrameShadow(QFrame::Sunken);
    config_widget_scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    config_widget_scroll_area_->setWidgetResizable(true);
    config_widget_scroll_area_->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
    setPlaceholderWidget();
    connect(plugins_list_view_->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &){
        try {
            const auto &p = plugin_registry_.plugins().at(current.data(Qt::UserRole).toString());
            config_widget_scroll_area_->setWidget(new PluginWidget(plugin_registry_, p));  // takes ownership
        }
        catch (const out_of_range &) {
            setPlaceholderWidget();
        }
    });

    // Layout

    auto *l = new QHBoxLayout(this);
    l->addWidget(plugins_list_view_);
    l->addWidget(config_widget_scroll_area_);
    l->setContentsMargins(6, 6, 6, 6);
    l->setSpacing(6);
}

PluginsWidget::~PluginsWidget() = default;

void PluginsWidget::tryShowPluginSettings(QString plugin_id)
{
    for (auto row = 0; row < proxy_model_->rowCount(); ++row)
    {
        if (auto index = proxy_model_->index(row, 0);
            index.data(Qt::UserRole).toString() == plugin_id)
        {
            plugins_list_view_->setCurrentIndex(index);
            plugins_list_view_->setFocus();
            return;
        }
    }
}

void PluginsWidget::showContextMenu(const QPoint &pos)
{
    QMenu menu;

    if (auto index = proxy_model_->mapToSource(plugins_list_view_->currentIndex()); index.isValid())
    {
        try {
            auto &p = plugin_registry_.plugins().at(index.data(Qt::UserRole).toString());
            auto id = p.id;

            if (p.metadata.load_type == User)
            {
                auto *a = new QAction(&menu);
                a->setText(p.enabled ? tr("Disable") : tr("Enable"));
                connect(a, &QAction::triggered,
                        this, [=, this] { plugin_registry_.setEnabledWithUserConfirmation(id, !p.enabled); });
                menu.addAction(a);

                if (p.state == Loaded)
                {
                    a = new QAction(&menu);
                    a->setText(tr("Unload"));
                    connect(a, &QAction::triggered,
                            this, [=, this] { plugin_registry_.setLoaded(id, false); });
                    menu.addAction(a);
                }

                if (p.state == Unloaded)
                {
                    a = new QAction(&menu);
                    a->setText(tr("Load"));
                    connect(a, &QAction::triggered,
                            this, [=, this] { plugin_registry_.setLoaded(id, true); });
                    menu.addAction(a);
                }

                menu.addSeparator();
            }
        }
        catch (const out_of_range &) { }
    }

    auto *a = new QAction(&menu);
    a->setText(tr("Enabled first"));
    a->setCheckable(true);
    a->setChecked(proxy_model_->showEnabledFirst());
    connect(a, &QAction::toggled, proxy_model_, &PluginsSortProxyModel::setShowEnabledFirst);
    menu.addAction(a);

    menu.exec(mapToGlobal(pos));
}

void PluginsWidget::setPlaceholderWidget()
{
    auto t = tr(
        "<p>Plugins are a community effort,<br>built by awesome people like you.</p>"
        "<p>💡 <a href='https://albertlauncher.github.io/gettingstarted/extension/'>"
        "Learn how to build plugins</a></p>"
        "<br>"  // move text slightly up, looks more balanced
        );

    auto *lbl = new QLabel(t);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setOpenExternalLinks(true);
    config_widget_scroll_area_->setWidget(lbl);  // takes ownership
}

void PluginsWidget::updatePluginListWidth()
{
    plugins_list_view_->setMaximumWidth(plugins_list_view_->sizeHintForColumn(0)
                                      + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
}
