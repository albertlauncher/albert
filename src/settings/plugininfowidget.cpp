// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/pluginprovider.h"
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

    auto *label = new QLabel(this);
    label->setText(QString("<a href=\"%1\">%1</a>").arg(spec.url));
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setFocusPolicy(Qt::NoFocus); // Remove ugly border
    label->setOpenExternalLinks(true);
    form_layout->addRow("Upstream:", label);

    form_layout->addRow("Maintainers:", new QLabel(spec.maintainers.join(", "), this));

    form_layout->addRow("Authors:", new QLabel(spec.authors.join("\n"), this));

    if (!spec.plugin_dependencies.isEmpty())
        form_layout->addRow("Plugin dependencies:", new QLabel(spec.plugin_dependencies.join(", "), this));

    if (!spec.runtime_dependencies.isEmpty())
        form_layout->addRow("Runtime dependencies:", new QLabel(spec.runtime_dependencies.join(", "), this));

    if (!spec.binary_dependencies.isEmpty())
        form_layout->addRow("Binary depencencies:", new QLabel(spec.binary_dependencies.join(", "), this));

    if (!spec.third_party.isEmpty())
        form_layout->addRow("Third party:", new QLabel(spec.third_party.join('\n'), this));

    QString type;
    switch (spec.type) {
        case PluginSpec::Type::None: type = "None"; break;
        case PluginSpec::Type::User: type = "User"; break;
        case PluginSpec::Type::Frontend: type = "Frontend"; break;
    }
    form_layout->addRow("Type:", new QLabel(type, this));


    form_layout->addRow("Provider:", new QLabel(spec.provider->name(), this));
}


enum class Column
{
    Enabled,
    State,
    Name,
    Version,
    Description,
};

PluginModel::PluginModel(albert::ExtensionRegistry &registry) : ExtensionWatcher<PluginProvider>(registry)
{
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
            || index.row() < 0 || rowCount(index.parent()) <= index.row()
            || index.column() < 0 || columnCount(index.parent()) <= index.column())
        return {};

    const auto &spec = *plugins[index.row()];

    switch (static_cast<Column>(index.column())) {
        case Column::Enabled:
            if (role == Qt::CheckStateRole && spec.type == PluginSpec::Type::User)
                return (spec.provider->isEnabled(spec.id)) ? Qt::Checked : Qt::Unchecked;
            break;

        case Column::State:
            if (role == Qt::DecorationRole)
                switch (spec.state) {
                    case PluginSpec::State::Error:
                        return QIcon(":plugin_error");
                    case PluginSpec::State::Loaded:
                        return QIcon(":plugin_loaded");
                    case PluginSpec::State::Unloaded:
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
    return {};
}

bool PluginModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (idx.isValid()
        && static_cast<Column>(idx.column()) == Column::Enabled
        && role == Qt::CheckStateRole){
        try {
            const auto &spec = *plugins[idx.row()];
            spec.provider->setEnabled(spec.id, value == Qt::Checked);
            emit dataChanged(index(idx.row(), (int)Column::Enabled),
                             index(idx.row(), (int)Column::State));
            return true;
        } catch (std::out_of_range &e){}
    }
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
    return {};
}

Qt::ItemFlags PluginModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0 || rowCount(index.parent()) <= index.row())
        return Qt::NoItemFlags;
    else if (plugins[index.row()]->type == PluginSpec::Type::User)
        return Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    else
        return Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled;
}

void PluginModel::onAdd(PluginProvider *pp)
{
    updatePlugins();
}

void PluginModel::onRem(PluginProvider *pp)
{
    updatePlugins();
}

void PluginModel::updatePlugins()
{
    beginResetModel();
    plugins.clear();
    for (const auto &[ppid, pp] : registry.extensions<PluginProvider>())
        for (const auto&[pid, spec] : pp->plugins())
            plugins.emplace_back(spec);

    std::sort(plugins.begin(), plugins.end(), [](const auto &l, const auto &r){ return l->name < r->name; });
    endResetModel();
}

PluginWidget::PluginWidget(albert::ExtensionRegistry &registry) : model(registry)
{
    setModel(&model);
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

    connect(this, &QTableView::activated, this, [this](const QModelIndex &index){
        auto *plugin_info_widget = new PluginInfoWidget(*model.plugins[index.row()]);
        plugin_info_widget->show();
    });

}
