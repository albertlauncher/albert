// Copyright (c) 2022-2024 Manuel Schneider

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
    FallbacksModel *fallbacks_model_;

};
