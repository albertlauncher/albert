// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "ui_querywidget.h"
#include <QWidget>
class QueryEngine;
class FallbacksModel;

class QueryWidget : public QWidget
{
public:

    QueryWidget(QueryEngine&);

private:

    void showEvent(QShowEvent *event) override;

    Ui::QueryWidget ui;
    QueryEngine &query_engine_;
    FallbacksModel *fallbacks_model_;

};
