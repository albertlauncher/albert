// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extensions/pluginprovider.h"
#include "pluginregistry.h"
#include "pluginwidget.h"
#include <QAbstractTableModel>
#include <QApplication>
#include <QFormLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSplitter>
#include <algorithm>
#include <map>
using namespace std;
using namespace albert;


class PluginModel: public QAbstractListModel
{
public:
    explicit PluginModel(PluginRegistry &plugin_registry) : plugin_registry_(plugin_registry)
    {
        connect(&plugin_registry, &PluginRegistry::pluginsChanged,
                this, &PluginModel::updatePlugins);
        updatePlugins();
    }

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

    int columnCount(const QModelIndex&) const override { return 1; }

    QVariant data(const QModelIndex &idx, int role) const override
    {
        if (!idx.isValid()
            || idx.row() < 0 || rowCount(idx.parent()) <= idx.row()
            || idx.column() < 0 || columnCount(idx.parent()) <= idx.column())
            return {};

        switch (const auto *p = plugins_[idx.row()]; role) {
        case Qt::CheckStateRole:
            if (p->metaData().user)
                return plugin_registry_.isEnabled(p->metaData().id) ? Qt::Checked : Qt::Unchecked;
            else
                return {};
            break;
        case Qt::DisplayRole:
            return p->metaData().name;
        case Qt::ForegroundRole:
            switch (p->state()) {
                case PluginState::Loaded:
                    return {};
                case PluginState::Unloaded:
                    return QColor(p->stateInfo().isEmpty() ? Qt::gray : Qt::red);
                case PluginState::Invalid:
                    return QColor(Qt::gray);
            }
                break;
        case Qt::ToolTipRole:
            return p->stateInfo();
        }
        return {};
    }

    bool setData(const QModelIndex &idx, const QVariant &value, int role) override
    {
        if (idx.isValid() && idx.column() == 0 && role == Qt::CheckStateRole){
            try {
                const auto *p = plugins_[idx.row()];
                plugin_registry_.enable(p->metaData().id, value == Qt::Checked);
                emit dataChanged(idx, idx);
                return true;
            } catch (std::out_of_range &e){}
        }
        return false;
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
    mutable std::map<QString, QIcon> icon_cache;
public:
    std::vector<const PluginLoader*> plugins_;
};



PluginWidget::PluginWidget(PluginRegistry &plugin_registry) : model_(new PluginModel(plugin_registry))
{
    setObjectName("PluginWidget");

    QSplitter *splitter = new QSplitter(this);
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setContentsMargins(splitter->handleWidth(), splitter->handleWidth(),
                                         splitter->handleWidth(), splitter->handleWidth());

    horizontalLayout->addWidget(splitter);

    listView_plugins = new QListView(this);
    listView_plugins->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listView_plugins->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView_plugins->setProperty("showDropIndicator", QVariant(false));
    listView_plugins->setAlternatingRowColors(true);
    listView_plugins->setSpacing(1);
    listView_plugins->setUniformItemSizes(true);
    listView_plugins->setModel(model_.get());
    listView_plugins->setMaximumWidth(listView_plugins->sizeHintForColumn(0)
                                      + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent));

    splitter->addWidget(listView_plugins);

    scrollArea_info = new QScrollArea(this);
    scrollArea_info->setFrameShape(QFrame::StyledPanel);
    scrollArea_info->setFrameShadow(QFrame::Sunken);
    scrollArea_info->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_info->setWidgetResizable(true);
    scrollArea_info->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);

    splitter->addWidget(scrollArea_info);

    connect(listView_plugins->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &PluginWidget::onUpdatePluginWidget);

    connect(model_.get(), &PluginModel::dataChanged,
            this, &PluginWidget::onUpdatePluginWidget);

    onUpdatePluginWidget();
}

PluginWidget::~PluginWidget() = default;

void PluginWidget::onUpdatePluginWidget()
{
    auto current = listView_plugins->currentIndex();
    QLabel *l;

    if (!current.isValid()){

        l = new QLabel("Select a plugin");
        l->setEnabled(false);
        l->setAlignment(Qt::AlignCenter);
        scrollArea_info->setWidget(l);
        return;
    }

    auto &p = *model_->plugins_[current.row()];
    auto *w = new QWidget;
    auto *vb = new QVBoxLayout;
    w->setLayout(vb);

    // Title and description
    vb->addWidget(new QLabel(
            QString("<span style=\"font-size:16pt;\">%1</span><br><span style=\"font-size:11pt;\">%2</span>")
                    .arg(p.metaData().name, p.metaData().description)
    ));

    // id, version, license, maintainers
    QString maint = p.metaData().maintainers.isEmpty()
                    ? "<span style=\"color:#c04040;\">This plugin is looking for a maintainer!</span>"
                    : "Maintained by " + p.metaData().maintainers.join(", ");
    l = new QLabel(QString("<span style=\"font-size:9pt;font-style:italic;color:#808080;\"><a href=\"%1\">%2 v%3</a> %4. %5</span>")
                           .arg(p.metaData().url, p.metaData().id, p.metaData().version, p.metaData().license, maint));
    l->setOpenExternalLinks(true);
    vb->addWidget(l);


    // Potentially long descrition
    if (auto t = p.metaData().long_description; !t.isEmpty()){
        l = new QLabel(p.metaData().long_description);
        l->setTextFormat(Qt::MarkdownText);
        l->setWordWrap(true);
        l->setOpenExternalLinks(true);
        vb->addWidget(l);
    }

    switch (p.state()) {
        case PluginState::Invalid:
            break;
        case PluginState::Unloaded:
            // Unloaded info
            if (!p.stateInfo().isEmpty()){
                l = new QLabel(p.stateInfo());
                l->setWordWrap(true);
                vb->addWidget(l);
            }
            break;
        case PluginState::Loaded:
            // Config widget
            if (auto *inst = p.instance(); inst)
                if (auto *cw = inst->buildConfigWidget())
                    vb->addWidget(cw, 1); // Strech=1
            break;
    }

    vb->addStretch();

    auto add_meta = [&](const QString &s){
        vb->addWidget(new QLabel(QString("<span style=\"font-size:9pt;color:#808080;\">%1</span>").arg(s)));
    };

    // Credits if any
    if (!p.metaData().third_party_credits.isEmpty())
        vb->addWidget(new QLabel(QString("<span style=\"font-size:9pt;color:#808080;\">Credits: %1</span>")
                                         .arg(p.metaData().third_party_credits.join(", "))));

    // Provider
    vb->addWidget(new QLabel(QString("<span style=\"font-size:9pt;color:#808080;\">%1, Interface id:  %2</span>")
                    .arg(p.provider()->name(), p.metaData().iid)));

    // Requirements
    if (!p.metaData().binary_dependencies.isEmpty())
        add_meta(QString("Required executable(s) in PATH: %1").arg(p.metaData().binary_dependencies.join(", ")));
    if (!p.metaData().runtime_dependencies.isEmpty())
        add_meta(QString("Required libraries: %1").arg(p.metaData().runtime_dependencies.join(", ")));

    // Path
    add_meta(p.path);

    scrollArea_info->setWidget(w);
}
