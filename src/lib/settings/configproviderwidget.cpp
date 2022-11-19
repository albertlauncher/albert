// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/configwidgetprovider.h"
#include "configproviderwidget.h"
#include <QHBoxLayout>
#include <utility>
#include <vector>
#include <QGroupBox>
using namespace albert;
using namespace std;

ConfigProviderWidget::ConfigProviderWidget(albert::ExtensionRegistry &registry):
        ExtensionWatcher<ConfigWidgetProvider>(registry)
{
    for (auto &[id, p] : registry.extensions<ConfigWidgetProvider>())
        providers.push_back(p);

    resetUI();

    list_widget.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
//    list_widget.setFrameShape(QFrame::NoFrame);

    auto *layout = new QHBoxLayout(this);
    auto *groupbox = new QGroupBox(this);
    groupbox->setLayout(new QVBoxLayout(this));
    groupbox->layout()->addWidget(&list_widget);
    groupbox->layout()->setContentsMargins(0,0,0,0);
    groupbox->layout()->setSpacing(0);
    groupbox->setTitle("Extensions");
    layout->addWidget(groupbox);
    layout->addWidget(&stacked_widget);
    layout->setStretch(0,0);
    layout->setStretch(1,1);
//    layout->setContentsMargins(0,0,0,0);
//    layout->setSpacing(0);
    setLayout(layout);

    connect(&list_widget, &QListWidget::currentRowChanged,
            &stacked_widget, &QStackedWidget::setCurrentIndex);
}


void ConfigProviderWidget::resetUI()
{
    list_widget.clear();
    while(stacked_widget.count() > 0){
        QWidget* widget = stacked_widget.widget(0);
        stacked_widget.removeWidget(widget);
        widget->deleteLater();
    }

    vector<pair<QString,QWidget*>> items;
    for (auto *cwp : providers){
        auto *widget = cwp->buildConfigWidget();
        items.emplace_back(cwp->name(), widget);
    }

    sort(items.begin(), items.end(), [](auto &l, auto &r){ return l.first < r.first; });

    for (auto &item : items){
        list_widget.addItem(item.first);
        stacked_widget.addWidget(item.second);
    }

    list_widget.setCurrentRow(0);
    stacked_widget.setCurrentIndex(0);

    // resize to contents
    list_widget.setMinimumWidth(list_widget.sizeHintForColumn(0));
    list_widget.setMaximumWidth(list_widget.sizeHintForColumn(0));
}

void ConfigProviderWidget::onAdd(ConfigWidgetProvider *t)
{
    resetUI();
}

void ConfigProviderWidget::onRem(ConfigWidgetProvider *t)
{
    resetUI();
}
//
//
//
//// Copyright (c) 2022 Manuel Schneider
//
//#include "albert/extensions/configwidgetprovider.h"
//#include "settingswidget.h"
//#include <QHBoxLayout>
//#include <QHeaderView>
//#include <vector>
//using namespace albert;
//using namespace std;
//
//ConfigProviderWidget::ConfigProviderWidget(albert::ExtensionRegistry &registry) : registry(registry)
//{
//    auto layout = new QHBoxLayout(this);
//    layout->addWidget(&tree_view);
//    layout->addWidget(&stacked_widget);
////    layout->setContentsMargins(0,0,0,0);
////    layout->setSpacing(0);
//    setLayout(layout);
//
//    tree_view.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
////    list_widget.setFrameShape(QFrame::NoFrame);
//
//    tree_view.setHeaderHidden(true);
//    tree_view.setIndentation(10);  // not that much
//    tree_view.setItemsExpandable(false);  // users must not be able to change the tree
//    tree_view.setRootIsDecorated(false);  // hide the arrows
//    tree_view.setSelectionBehavior(QAbstractItemView::SelectRows); // select full rows
//    tree_view.setModel(&tree_model);
//    resetTreeModel();
//
//    connect(&tree_view, &QTreeView::clicked, this, [this](const QModelIndex &index){
//        if (index.parent().isValid()){ // do nothing on click on groups
//            auto *item = tree_model.itemFromIndex(index);
//            stacked_widget.setCurrentIndex(item->data().toInt());
//            qDebug() << index;
//        }
//    });
//}
//
//
//void ConfigProviderWidget::resetTreeModel()
//{
//    // Clear
//    tree_model.clear();
//    while(stacked_widget.count()){
//        QWidget* widget = stacked_widget.widget(0);
//        stacked_widget.removeWidget(widget);
//        widget->deleteLater();
//    }
//
//    QStandardItem *root = tree_model.invisibleRootItem();
//
//    auto general_item = new QStandardItem(tr("General"));
//    general_item->setFlags(Qt::ItemIsEnabled);
//    root->appendRow(general_item);
//
//    auto extensions_item = new QStandardItem(tr("Extensions"));
//    extensions_item->setFlags(Qt::ItemIsEnabled);
//    root->appendRow(extensions_item);
//
//    for (auto *provider : extensions()){
//        auto *widget = provider->buildConfigWidget();
//        auto stack_index = stacked_widget.addWidget(widget);
//        auto *item = new QStandardItem(provider->configTitle());
//        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
//        item->setData(stack_index);
//        switch (provider->configGroup()) {
//            case ConfigGroup::General:
//                general_item->appendRow(item);
//                break;
//            case ConfigGroup::Extension:
//                extensions_item->appendRow(item);
//                break;
//        }
//    }
//
//    tree_view.expandAll();
//
//    // Adjust to contents
//    tree_view.header()->setStretchLastSection(false);  // needed to get proper column width for resize
//    tree_view.resizeColumnToContents(0);  //
//    tree_view.setMaximumWidth(tree_view.header()->sectionSize(0)+tree_view.indentation());
//    tree_view.header()->setStretchLastSection(true); // now turn it on again for full row selection
//
//    general_item->sortChildren(0);
//    extensions_item->sortChildren(0);
//}
//
//void ConfigProviderWidget::onAdd(ConfigWidgetProvider *t)
//{
//    resetTreeModel();
//}
//
//void ConfigProviderWidget::onRem(ConfigWidgetProvider *t)
//{
//    resetTreeModel();
//}
