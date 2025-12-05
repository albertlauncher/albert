// Copyright (c) 2022-2025 Manuel Schneider

#include "fallbacksmodel.h"
#include "queryengine.h"
#include "queryhandlermodel.h"
#include "querywidget.h"
#include "usagescoring.h"
#include <QHeaderView>

QueryWidget::QueryWidget(QueryEngine &query_engine) : query_engine_(query_engine)
{
    ui.setupUi(this);

    auto usage_scoring = query_engine_.usageScoring();

    ui.slider_decay->setValue((int)(usage_scoring.memory_decay * 100));

    connect(ui.slider_decay, &QSlider::sliderReleased, this,
            [this]{ query_engine_.setMemoryDecay((double)ui.slider_decay->value()/100.0); });

    ui.checkBox_prioritizePerfectMatch->setChecked(usage_scoring.prioritize_perfect_match);

    connect(ui.checkBox_prioritizePerfectMatch, &QCheckBox::toggled, this,
            [this](bool val){ query_engine_.setPrioritizePerfectMatch(val); });

    ui.tableView_queryHandlers->setModel(new QueryHandlerModel(query_engine_, this)); // Takes ownership
    ui.tableView_fallbackOrder->setModel(fallbacks_model_ = new FallbacksModel(query_engine_, this)); // Takes ownership

    for (auto *tv : {ui.tableView_queryHandlers, ui.tableView_fallbackOrder})
    {
        tv->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);  // Requires a model!
        tv->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);  // Requires a model!
        tv->horizontalHeader()->setSectionsClickable(false);
    }

    // Fix for some styles on linux setting a minimum secion size
    ui.tableView_queryHandlers->horizontalHeader()->setMinimumSectionSize(0);

    // Width adjust
    auto updateWidth = [&]{
        int width = 0;
        for (int c = 0; c < ui.tableView_queryHandlers->model()->columnCount(); ++c)
            width += ui.tableView_queryHandlers->horizontalHeader()->sectionSize(c);
        width += + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        ui.tableView_queryHandlers->setFixedWidth(width + 2);  // 2: Frame spacing?
    };
    connect(&query_engine_, &QueryEngine::queryHandlerAdded, this, updateWidth);
    connect(&query_engine_, &QueryEngine::queryHandlerRemoved, this, updateWidth);
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
