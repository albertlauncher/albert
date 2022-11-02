// Copyright (c) 2022 Manuel Schneider

#include "albert/pluginprovider.h"
#include "pluginwidget.h"
#include "albert/albert.h"
#include <QFormLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPlainTextEdit>
#include <algorithm>
using namespace albert;
using namespace std;

PluginInfoWidget::PluginInfoWidget(const albert::PluginSpec &spec)
{
//    setStyleSheet("border: 1px solid red");

    auto *form_layout = new QFormLayout();

//    setWindowFlags(Qt::Popup|Qt:: Dialog);
//    setWindowModality(Qt::WindowModal);


//    form_layout->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
//    form_layout->setContentsMargins(0,0,0,0);

    setLayout(form_layout);

    static const std::vector<const QString> STATE_NAMES = {
            "Invalid",
            "Unloaded",
            "Loading",
            "Loaded",
            "Error"
    };

    form_layout->addRow("Identifier:", new QLabel(spec.id, this));

    form_layout->addRow("Interface:", new QLabel(spec.iid, this));

    form_layout->addRow("Version:", new QLabel(spec.version, this));

    form_layout->addRow("Path:", new QLabel(spec.path, this));

    form_layout->addRow("Name:", new QLabel(spec.name, this));

    form_layout->addRow("Description:", new QLabel(spec.description, this));

    form_layout->addRow("License:", new QLabel(spec.license, this));

    QLabel *label = new QLabel(this);
    label->setText(QString("<a href=\"%1\">%1</a>").arg(spec.url));
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setFocusPolicy(Qt::NoFocus); // Remove ugly border
    label->setOpenExternalLinks(true);
    form_layout->addRow("Upstream:", label);

    form_layout->addRow("Maintainers:", new QLabel(spec.maintainers.join(", "), this));

    form_layout->addRow("Authors:", new QLabel(spec.authors.join("\n"), this));

    form_layout->addRow("Plugin dependencies:", new QLabel(spec.plugin_dependencies.join(", "), this));

    form_layout->addRow("Runtime dependencies:", new QLabel(spec.runtime_dependencies.join(", "), this));

    form_layout->addRow("Binary depencencies:", new QLabel(spec.binary_dependencies.join(", "), this));

    form_layout->addRow("Third party:", new QLabel(spec.third_party.join('\n'), this));

    QString type;
    switch (spec.type) {
        case PluginType::None: type = "None"; break;
        case PluginType::User: type = "User"; break;
        case PluginType::Frontend: type = "Frontend"; break;
    }
    form_layout->addRow("Type:", new QLabel(type, this));


    form_layout->addRow("Provider:", new QLabel(spec.provider->id(), this));
}


enum class Column
{
    Enabled,
    State,
    Name,
    Version,
    Description,
};

PluginModel::PluginModel()
{
    for (auto &[id, pp] : albert::extensionRegistry().extensionsOfType<PluginProvider>())
        connect(pp, &PluginProvider::pluginStateChanged,this, &PluginModel::onPluginStateChanged);
    updatePlugins();
}

int PluginModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(plugins.size());
}

int PluginModel::columnCount(const QModelIndex &) const
{
    return 5;
}

QVariant PluginModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
            || index.row() < 0 || rowCount() <= index.row()
            || index.column() < 0 || columnCount() <= index.column())
        return QVariant();

    const auto &spec = *plugins[index.row()];

    switch (static_cast<Column>(index.column())) {
        case Column::Enabled:
            if (role == Qt::CheckStateRole && spec.type == PluginType::User)
                return (spec.provider->isEnabled(spec.id)) ? Qt::Checked : Qt::Unchecked;
            break;

        case Column::State:
            if (role == Qt::DecorationRole)
                switch (spec.state) {
                    case PluginState::Error:
                        return QIcon(":plugin_error");
                    case PluginState::Loaded:
                        return QIcon(":plugin_loaded");
                    case PluginState::Loading:
                        return QIcon(":plugin_loading");
                    case PluginState::Unloaded:
                        return QIcon(":plugin_unloaded");
                }
            else if (role == Qt::ToolTipRole && !spec.reason.isEmpty())
                return QString("<span style=\"color:#ff0000;\">%1</span>").arg(spec.reason);
            break;

        case Column::Name:
            if (role == Qt::DisplayRole)
                return spec.name;
            else if (role == Qt::DecorationRole)
                return spec.provider->icon();
            break;

        case Column::Version:
            if (role == Qt::DisplayRole)
                return spec.version;
            break;

        case Column::Description:
            if (role == Qt::DisplayRole)
                return spec.description;
            break;
    }
    return QVariant();
}

bool PluginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || static_cast<Column>(index.column()) != Column::Enabled || role != Qt::CheckStateRole)
        return false;

    const auto &spec = *plugins[index.row()];

    try {
        spec.provider->setEnabled(spec.id, value == Qt::Checked);
        return true;
    } catch (std::out_of_range &e){}
    return false;
}

QVariant PluginModel::headerData(int section, Qt::Orientation orientation, int role) const
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
    return QVariant();
}

Qt::ItemFlags PluginModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return Qt::NoItemFlags;
    else if (plugins[index.row()]->type == PluginType::User)
        return Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    else
        return Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled;
}

void PluginModel::onAdd(PluginProvider *pp)
{
    connect(pp, &PluginProvider::pluginStateChanged,
            this, &PluginModel::onPluginStateChanged);
    updatePlugins();
}

void PluginModel::onRem(PluginProvider *pp)
{
    disconnect(pp, &PluginProvider::pluginStateChanged,
               this, &PluginModel::onPluginStateChanged);
    updatePlugins();
}

void PluginModel::updatePlugins()
{
    beginResetModel();

    plugins.clear();
    for (const auto &[ppid, pp] : albert::extensionRegistry().extensionsOfType<PluginProvider>())
        for (const auto&[pid, spec] : pp->plugins())
            plugins.emplace_back(&spec);

    std::sort(plugins.begin(), plugins.end(), [](const auto &l, const auto &r){ return l->name < r->name; });

    endResetModel();
}

void PluginModel::onPluginStateChanged(const albert::PluginSpec &spec)
{
    if (auto it = std::find_if(plugins.cbegin(), plugins.cend(), [&](const auto *p){ return p->id == spec.id; });
        it != plugins.cend())
        dataChanged(index(it - plugins.cbegin(), 0), index(it - plugins.cbegin(), columnCount()));
}


PluginWidget::PluginWidget()
{
    setModel(&model);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlternatingRowColors(true);
    setShowGrid(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setFrameShape(QFrame::NoFrame);

    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(20);
    verticalHeader()->hide();

    horizontalHeader()->setFrameShape(QFrame::NoFrame);
    horizontalHeader()->setSectionsClickable(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode((int)Column::Description, QHeaderView::Stretch);
    horizontalHeader()->setVisible(true);

//    horizontalHeader()->setSectionResizeMode((int)Column::Authors, QHeaderView::Stretch);
//    horizontalHeader()->setStretchLastSection(true);
//    resizeRowsToContents();
//    resizeColumnsToContents();

//    layout()->setMargin(0);

//    setContentsMargins(0,0,0,0);

    connect(this, &QTableView::activated, this, [this](const QModelIndex &index){
        auto *plugin_info_widget = new PluginInfoWidget(*model.plugins[index.row()]);
        plugin_info_widget->show();
    });

}
