// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <QString>
#include <memory>
class QWidget;
class QueryEngine;
class App;

namespace albert
{
class Query;

/// The interface for albert frontends
class ALBERT_EXPORT Frontend : virtual public albert::Extension
{
public:
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;

    virtual QString input() const = 0;
    virtual void setInput(const QString&) = 0;

    virtual QWidget *createFrontendConfigWidget() = 0;

    std::shared_ptr<Query> query(const QString &query) const;

private:
    void setEngine(QueryEngine*);  // sure private?
    QueryEngine *query_engine;
    friend class ::App;
};

}
