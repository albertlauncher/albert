// Copyright (c) 2022-2024 Manuel Schneider

#include "fallbacksmodel.h"
#include "queryengine.h"
#include "queryhandlermodel.h"
#include "querywidget.h"
#include "usagedatabase.h"
#include <QHeaderView>

QueryWidget::QueryWidget(QueryEngine &qe)
{
    ui.setupUi(this);

    ui.slider_decay->setValue((int)(UsageHistory::memoryDecay() * 100));
    QObject::connect(ui.slider_decay, &QSlider::valueChanged, this,
                     [](int val){ UsageHistory::setMemoryDecay((double)val/100.0); });

    ui.checkBox_prioritizePerfectMatch->setChecked(UsageHistory::prioritizePerfectMatch());
    QObject::connect(ui.checkBox_prioritizePerfectMatch, &QCheckBox::toggled, this,
                     [](bool val){ UsageHistory::setPrioritizePerfectMatch(val); });

    ui.checkBox_runEmptyQuery->setChecked(qe.runEmptyQuery());
    QObject::connect(ui.checkBox_runEmptyQuery, &QCheckBox::toggled, this,
                     [&qe](bool val){ qe.setRunEmptyQuery(val); });

    for (auto *tv : {ui.tableView_queryHandlers, ui.tableView_fallbackOrder})
    {
        tv->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tv->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tv->horizontalHeader()->setSectionsClickable(false);
    }

    ui.tableView_queryHandlers->setModel(new QueryHandlerModel(qe, this)); // Takes ownership
    ui.tableView_fallbackOrder->setModel(new FallbacksModel(qe, this)); // Takes ownership

    // Size adjust does not work properly on macos do it manually
    auto updateWidth = [&]{
        int width = 0;
        for (int c = 0; c < ui.tableView_queryHandlers->model()->columnCount(); ++c)
            width += ui.tableView_queryHandlers->horizontalHeader()->sectionSize(c);

        ui.tableView_queryHandlers->setFixedWidth(width + 2);  // 2: Frame spacing?
    };
    connect(&qe, &QueryEngine::handlersChanged, this, updateWidth);
    updateWidth();

}
