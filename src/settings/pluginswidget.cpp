// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/pluginprovider/pluginprovider.h"
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
                        ? tr("This plugin is looking for a maintainer!")
                        : tr("Maintained by ") + p.metaData().maintainers.join(", ");
    l = new QLabel(QString("<span style=\"font-size:9pt;font-style:italic;color:#808080;\"><a href=\"%1\">%2 v%3</a> %4. %5</span>")
                       .arg(p.metaData().url, p.metaData().id, p.metaData().version, p.metaData().license, maint));
    l->setOpenExternalLinks(true);
    vb->addWidget(l);

    if (p.state() == PluginState::Loaded) {
        // Config widget
        if (auto *inst = p.instance(); inst)
            if (auto *cw = inst->buildConfigWidget())
                vb->addWidget(cw, 1); // Strech=1
    } else if (!p.stateInfo().isEmpty()){
        // Unloaded info
        if (!p.stateInfo().isEmpty()){
            l = new QLabel(p.stateInfo());
            l->setWordWrap(true);
            vb->addWidget(l);
        }
    }

    vb->addStretch();

    // List extensions
    if (p.state() == PluginState::Loaded) {
        if (auto extensions = p.instance()->extensions(); !extensions.empty()) {
            QStringList exts;
            for (auto *e : extensions)
                exts << QString("%1 (%2)").arg(e->name(), e->description());
            vb->addWidget(new QLabel(tr("<span style=\"font-size:9pt;color:#808080;\">Provides: %1</span>").arg(exts.join(", "))));
        }
    }

    auto add_meta = [&](const QString &s){
        vb->addWidget(new QLabel(QString("<span style=\"font-size:9pt;color:#808080;\">%1</span>").arg(s)));
    };

    // Credits if any
    if (!p.metaData().third_party_credits.isEmpty())
        vb->addWidget(new QLabel(QString("<span style=\"font-size:9pt;color:#808080;\">Credits: %1</span>")
                                         .arg(p.metaData().third_party_credits.join(", "))));

    // Provider
    vb->addWidget(new QLabel(QString("<span style=\"font-size:9pt;color:#808080;\">%1, Interface id:  %2</span>")
                    .arg(p.provider().name(), p.metaData().iid)));

    // Requirements
    if (!p.metaData().binary_dependencies.isEmpty())
        add_meta(tr("Required executable(s) in PATH: %1").arg(p.metaData().binary_dependencies.join(", ")));
    if (!p.metaData().runtime_dependencies.isEmpty())
        add_meta(tr("Required libraries: %1").arg(p.metaData().runtime_dependencies.join(", ")));

    // Path
    add_meta(p.path);

    scrollArea_info->setWidget(w);
}
