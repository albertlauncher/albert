// Copyright (c) 2022 Manuel Schneider

#include "settingswidget.h"
#include "app.h"
#include <QHBoxLayout>
#include <vector>
#include <utility>
#include "settingswidgetprovider.h"
using namespace std;
using namespace albert;

SettingsWidget::SettingsWidget(App &app) : app(app)
{
    resetUI();

    list_widget.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
    list_widget.setFrameShape(QFrame::NoFrame);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(&list_widget);
    layout->addWidget(&stacked_widget);
    layout->setStretch(0,0);
    layout->setStretch(1,1);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    setLayout(layout);

    connect(&list_widget, &QListWidget::currentRowChanged,
            &stacked_widget, &QStackedWidget::setCurrentIndex);
}


void SettingsWidget::resetUI()
{
    list_widget.clear();
    while(stacked_widget.count() > 0){
        QWidget* widget = stacked_widget.widget(0);
        stacked_widget.removeWidget(widget);
        widget->deleteLater();
    }

    vector<pair<QString,QWidget*>> items;
    for (auto *swp : ExtensionWatcher<SettingsWidgetProvider>::extensions()){
        auto *widget = swp->createSettingsWidget();
        items.emplace_back(widget->objectName(), widget);
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

void SettingsWidget::onAdd(SettingsWidgetProvider *t)
{
    resetUI();
}

void SettingsWidget::onRem(SettingsWidgetProvider *t)
{
    resetUI();
}
