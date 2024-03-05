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

    for (auto *tv : {ui.tableView_queryHandlers, ui.tableView_fallbackOrder})
    {
        tv->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tv->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tv->horizontalHeader()->setSectionsClickable(false);
    }

    ui.tableView_queryHandlers->setModel(new QueryHandlerModel(qe, this)); // Takes ownership
    ui.tableView_fallbackOrder->setModel(fallbacks_model_ = new FallbacksModel(qe, this)); // Takes ownership

    // Fix for some styles on linux setting a minimum secion size
    ui.tableView_queryHandlers->horizontalHeader()->setMinimumSectionSize(0);

    // Size adjust does not work properly on macos do it manually
    auto updateWidth = [&]{
        int width = 0;
        for (int c = 0; c < ui.tableView_queryHandlers->model()->columnCount(); ++c)
            width += ui.tableView_queryHandlers->horizontalHeader()->sectionSize(c);
        width += + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        ui.tableView_queryHandlers->setFixedWidth(width + 2);  // 2: Frame spacing?
    };
    connect(&qe, &QueryEngine::handlerAdded, this, updateWidth);
    connect(&qe, &QueryEngine::handlerRemoved, this, updateWidth);
    updateWidth();
}

void QueryWidget::showEvent(QShowEvent*)
{
    // This is a workaround such that FallbackHandlers dont need a signal
    // for the change of fallbacks. This is a bit dirty, but a cheap solution
    // maintenance and performance wise. The alternative would be to have a
    // signal in FallbackHandler and connect change the fallbacksmodel to
    // listen to it. That would be a lot of overhead for a very simple thing.
    fallbacks_model_->updateFallbackList();
}
