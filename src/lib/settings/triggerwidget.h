// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QWidget>
namespace albert{ class ExtensionRegistry; }
class QueryEngine;

class TriggerWidget : public QWidget
{
public:
    explicit TriggerWidget(albert::ExtensionRegistry&, QueryEngine &);
};
