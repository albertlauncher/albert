// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/configwidgetprovider.h"
#include "settingswidget.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <vector>
using namespace albert;
using namespace std;

SettingsWidget::SettingsWidget(albert::ExtensionRegistry &registry) : registry(registry)
{
    auto layout = new QHBoxLayout(this);
    layout->addWidget(&tree_view);
    layout->addWidget(&stacked_widget);
//    layout->setContentsMargins(0,0,0,0);
//    layout->setSpacing(0);
    setLayout(layout);

    tree_view.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
//    list_widget.setFrameShape(QFrame::NoFrame);

    tree_view.setHeaderHidden(true);
    tree_view.setIndentation(10);  // not that much
    tree_view.setItemsExpandable(false);  // users must not be able to change the tree
    tree_view.setRootIsDecorated(false);  // hide the arrows
    tree_view.setSelectionBehavior(QAbstractItemView::SelectRows); // select full rows
    tree_view.setModel(&tree_model);
    resetTreeModel();

    connect(&tree_view, &QTreeView::clicked, this, [this](const QModelIndex &index){
        if (index.parent().isValid()){ // do nothing on click on groups
            auto *item = tree_model.itemFromIndex(index);
            stacked_widget.setCurrentIndex(item->data().toInt());
            qDebug() << index;
        }
    });
}


void SettingsWidget::resetTreeModel()
{
    // Clear
    tree_model.clear();
    while(stacked_widget.count()){
        QWidget* widget = stacked_widget.widget(0);
        stacked_widget.removeWidget(widget);
        widget->deleteLater();
    }

    QStandardItem *root = tree_model.invisibleRootItem();

    auto general_item = new QStandardItem(tr("General"));
    general_item->setFlags(Qt::ItemIsEnabled);
    root->appendRow(general_item);

    auto extensions_item = new QStandardItem(tr("Extensions"));
    extensions_item->setFlags(Qt::ItemIsEnabled);
    root->appendRow(extensions_item);

    for (auto *provider : extensions()){
        auto *widget = provider->buildConfigWidget();
        auto stack_index = stacked_widget.addWidget(widget);
        auto *item = new QStandardItem(provider->configTitle());
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        item->setData(stack_index);
        switch (provider->configGroup()) {
            case ConfigGroup::General:
                general_item->appendRow(item);
                break;
            case ConfigGroup::Extension:
                extensions_item->appendRow(item);
                break;
        }
    }

    tree_view.expandAll();

    // Adjust to contents
    tree_view.header()->setStretchLastSection(false);  // needed to get proper column width for resize
    tree_view.resizeColumnToContents(0);  //
    tree_view.setMaximumWidth(tree_view.header()->sectionSize(0)+tree_view.indentation());
    tree_view.header()->setStretchLastSection(true); // now turn it on again for full row selection

    general_item->sortChildren(0);
    extensions_item->sortChildren(0);
}

void SettingsWidget::onAdd(ConfigWidgetProvider *t)
{
    resetTreeModel();
}

void SettingsWidget::onRem(ConfigWidgetProvider *t)
{
    resetTreeModel();
}
