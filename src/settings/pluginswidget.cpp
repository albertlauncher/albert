// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/pluginprovider/pluginprovider.h"
#include "pluginregistry.h"
#include "pluginsmodel.h"
#include "pluginswidget.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QScrollArea>
#include <QSplitter>
using namespace albert;
using namespace std;


PluginsWidget::PluginsWidget(PluginRegistry &plugin_registry) : model_(new PluginsModel(plugin_registry))
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
            this, &PluginsWidget::onUpdatePluginWidget);

    connect(model_.get(), &PluginsModel::dataChanged,
            this, &PluginsWidget::onUpdatePluginWidget);

    onUpdatePluginWidget();
}

void PluginsWidget::tryShowPluginSettings(QString plugin_id)
{
    for (auto row = 0; row < model_->rowCount(); ++row)
        if (auto index = model_->index(row); index.data(Qt::UserRole).toString() == plugin_id){
            listView_plugins->setCurrentIndex(index);
            listView_plugins->setFocus();
        }
}

PluginsWidget::~PluginsWidget() = default;

void PluginsWidget::onUpdatePluginWidget()
{
    auto current = listView_plugins->currentIndex();
    QLabel *l;

    if (!current.isValid()){
        l = new QLabel(tr("Select a plugin"));
        l->setAlignment(Qt::AlignCenter);
        scrollArea_info->setWidget(l);
        return;
    }

    auto &p = *model_->plugins_[current.row()];
    auto *widget = new QWidget;
    auto *vl = new QVBoxLayout;
    widget->setLayout(vl);

    // // Title
    // vl->addWidget(new QLabel(QString("<span style=\"font-size:16pt;font-style:bold;\">%1</span>").arg(p.metaData().name)));

    // // Description
    // vl->addWidget(new QLabel(QString("<span style=\"font-size:11pt;font-style:italic;\">%1</span>").arg(p.metaData().description)));

    vl->addWidget(new QLabel(QString("<span style=\"font-size:16pt;font-weight:600;\">%1</span><br>"
                                     "<span style=\"font-size:11pt;font-weight:lighter;font-style:italic;\">%2</span>")
                                               .arg(p.metaData().name, p.metaData().description)));

    // Plugin specific
    if (p.state() == Plugin::State::Loaded)
    {
        // Config widget
        if (auto *inst = p.instance(); inst)
            if (auto *cw = inst->buildConfigWidget())
                vl->addWidget(cw, 1); // Strech=1
    }
    else if (!p.stateInfo().isEmpty())
    {
        // Unloaded info
        if (!p.stateInfo().isEmpty())
        {
            l = new QLabel(p.stateInfo());
            l->setWordWrap(true);
            vl->addWidget(l);
        }
    }

    vl->addStretch();

    // META INFO

    QStringList meta;

    // Credits if any
    if (auto list = p.metaData().third_party_credits; !list.isEmpty())
        meta << tr("Credits: %1").arg(list.join(", "));

    // Required executables, if any
    if (auto list = p.metaData().binary_dependencies; !list.isEmpty())
        meta << tr("Required executables: %1", nullptr, list.size()).arg(list.join(", "));

    // Required libraries, if any
    if (auto list = p.metaData().runtime_dependencies; !list.isEmpty())
        meta << tr("Required libraries: %1", nullptr, list.size()).arg(list.join(", "));

    // Id, version, license, authors
    QStringList authors;
    for (const auto &author : p.metaData().authors)
        if (author.startsWith(QStringLiteral("@")))
            authors << QStringLiteral("<a href=\"https://github.com/%1\">%2</a>")
                           .arg(author.mid(1), author);
        else
            authors << author;

    meta << QString("<span style=\"color:#808080;\"><a href=\"%1\">%2 v%3</a>. %4. %5.</span>")
                       .arg(p.metaData().url,
                            p.metaData().id,
                            p.metaData().version,
                            tr("License: %1").arg(p.metaData().license),
                            tr("Authors: %1", nullptr, authors.size()).arg(authors.join(", ")));

    // Provider
    meta << tr("%1, Interface: %2").arg(p.provider->name(), p.metaData().iid);

    // Path
    meta << p.path();

    // Add meta
    l = new QLabel(QString("<span style=\"font-size:9pt;color:#808080;\">%1</span>").arg(meta.join("<br>")));
    l->setOpenExternalLinks(true);
    l->setWordWrap(true);
    vl->addWidget(l);

    scrollArea_info->setWidget(widget);
}
