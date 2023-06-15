// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "ui_globalsearchwidget.h"
namespace albert { class ExtensionRegistry; }
class QueryEngine;

class GlobalSearchWidget : public QWidget
{
public:
    explicit GlobalSearchWidget(QueryEngine&, albert::ExtensionRegistry&);
public:
    void updateGlobalHandlerList();
    void updateFallbackHandlerList();
    Ui::GlobalSearchWidget ui;
    QueryEngine &engine;
};
