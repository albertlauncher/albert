// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QTableView>
class QueryEngine;
class TriggerModel;

class TriggerWidget : public QTableView
{
public:
    explicit TriggerWidget(QueryEngine &);
    ~TriggerWidget() override;

    std::unique_ptr<TriggerModel> model;
};
