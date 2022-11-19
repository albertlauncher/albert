// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QTableView>
namespace albert{ class ExtensionRegistry; }
class QueryEngine;
class TriggerModel;

class TriggerWidget : public QTableView
{
public:
    explicit TriggerWidget(albert::ExtensionRegistry&, QueryEngine &);
    ~TriggerWidget() override;

    std::unique_ptr<TriggerModel> model;
};
