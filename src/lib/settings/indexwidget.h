// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QWidget>
namespace albert{ class ExtensionRegistry; }
class QueryEngine;

class IndexWidget : public QWidget
{
public:
    explicit IndexWidget(albert::ExtensionRegistry&, QueryEngine &);
private:

};
