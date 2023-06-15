// Copyright (c) 2022-2023 Manuel Schneider

#include "queryengine.h"
#include "globalsearchwidget.h"
#include "albert/extensionregistry.h"
using namespace albert;
using namespace std;

GlobalSearchWidget::GlobalSearchWidget(QueryEngine &qe, ExtensionRegistry &registry)
    : engine(qe)
{
    ui.setupUi(this);

    ui.checkBox_fuzzy->setChecked(engine.fuzzy());
    QObject::connect(ui.checkBox_fuzzy, &QCheckBox::toggled, this,
                     [this](bool checked){ engine.setFuzzy(checked); });

    ui.lineEdit_separators->setText(engine.separators());
    QObject::connect(ui.lineEdit_separators, &QLineEdit::editingFinished, this,
                     [this](){ engine.setSeparators(ui.lineEdit_separators->text()); });

    ui.slider_decay->setValue((int)(engine.memoryDecay()*100));
    QObject::connect(ui.slider_decay, &QSlider::valueChanged, this,
                     [this](int val){ engine.setMemoryDecay((double)val/100.0); });

    ui.checkBox_prioritizePerfectMatch->setChecked(engine.prioritizePerfectMatch());
    QObject::connect(ui.checkBox_prioritizePerfectMatch, &QCheckBox::toggled, this,
                     [this](bool val){ engine.setPrioritizePerfectMatch(val); });


    connect(&registry, &ExtensionRegistry::added, this, [this](Extension *e){
        if (dynamic_cast<GlobalQueryHandler*>(e))
            updateGlobalHandlerList();
        else if (dynamic_cast<FallbackHandler*>(e))
            updateFallbackHandlerList();
    });

    connect(&registry, &ExtensionRegistry::removed, this, [this](Extension *e){
        if (dynamic_cast<GlobalQueryHandler*>(e))
            updateGlobalHandlerList();
        else if (dynamic_cast<FallbackHandler*>(e))
            updateFallbackHandlerList();
    });


    connect(ui.listWidget_globalHandlers, &QListWidget::itemChanged, this, [this](QListWidgetItem *item){
        auto *handler = static_cast<albert::GlobalQueryHandler*>(item->data(Qt::UserRole).value<void*>());
        engine.setEnabled(handler, item->checkState() == Qt::Checked);
    });

    connect(ui.listWidget_fallbackHandlers, &QListWidget::itemChanged, this, [this](QListWidgetItem *item){
        auto *handler = static_cast<albert::FallbackHandler*>(item->data(Qt::UserRole).value<void*>());
        engine.setEnabled(handler, item->checkState() == Qt::Checked);
    });

    updateGlobalHandlerList();
    updateFallbackHandlerList();
}

void GlobalSearchWidget::updateGlobalHandlerList()
{
    ui.listWidget_globalHandlers->clear();
    for (auto * handler : engine.globalHandlers()){
        auto *item = new QListWidgetItem;
        item->setText(handler->name());
        item->setToolTip(QString("%1\n%2").arg(handler->id(), handler->description()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(engine.isEnabled(handler) ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(handler)));
        ui.listWidget_globalHandlers->addItem(item);
    }
    ui.listWidget_globalHandlers->sortItems();
}

void GlobalSearchWidget::updateFallbackHandlerList()
{
    ui.listWidget_fallbackHandlers->clear();
    for (auto * handler : engine.fallbackHandlers()){
        auto *item = new QListWidgetItem;
        item->setText(handler->name());
        item->setToolTip(QString("%1\n%2").arg(handler->id(), handler->description()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(engine.isEnabled(handler) ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(handler)));
        ui.listWidget_fallbackHandlers->addItem(item);
    }
    ui.listWidget_fallbackHandlers->sortItems();
}



