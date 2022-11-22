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

    list_widget.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
//    list_widget.setFrameShape(QFrame::VLine);
//    list_widget.setFrameStyle(QFrame::VLine | QFrame::Plain);
    list_widget.setAlternatingRowColors(true);
    list_widget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    auto *layout = new QHBoxLayout(this);
    layout->addWidget(&list_widget);
    layout->addWidget(&stacked_widget);
    layout->setStretch(0,0);
    layout->setStretch(1,1);
    layout->setContentsMargins(12,12,12,12);
//    layout->setSpacing(0);
    setLayout(layout);

    connect(&list_widget, &QListWidget::currentRowChanged,
            &stacked_widget, &QStackedWidget::setCurrentIndex);
    resetUI();
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
