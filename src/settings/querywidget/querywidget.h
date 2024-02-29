// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_querywidget.h"
class QueryEngine;

class QueryWidget : public QWidget
{
public:

    QueryWidget(QueryEngine&);

private:

    Ui::QueryWidget ui;

};
