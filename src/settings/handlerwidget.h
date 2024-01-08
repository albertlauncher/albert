// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include <QTableView>
namespace albert { class ExtensionRegistry; }
class QueryEngine;

class HandlerWidget : public QTableView
{
public:
    explicit HandlerWidget(QueryEngine&, albert::ExtensionRegistry&, QWidget *parent = nullptr);
    ~HandlerWidget() override;
};
