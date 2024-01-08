// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include <QTableView>
#include <memory>
namespace albert { class ExtensionRegistry; }
class QueryEngine;

class FallbackWidget : public QTableView
{
public:
    explicit FallbackWidget(QueryEngine&, albert::ExtensionRegistry&, QWidget *parent = nullptr);
    ~FallbackWidget() override;
};
