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

/// The interface for albert frontends.
class ALBERT_EXPORT Frontend : virtual public albert::Extension
{
public:
    /// Visibility of the frontend
    virtual bool isVisible() const = 0;

    /// Set the visibility state of the frontend
    virtual void setVisible(bool visible) = 0;

    /// Input line text
    virtual QString input() const = 0;

    /// Input line text setter
    virtual void setInput(const QString&) = 0;

    /// The native window id. Used to apply platform quirks.
    virtual unsigned long long winId() const = 0;

    /// The config widget show in the window settings tab
    virtual QWidget *createFrontendConfigWidget() = 0;

    /// The query object factory
    /// @note the QueryEngine is not available in the constructor. Do not use before albert:PluginInstance::initialize
    std::shared_ptr<Query> query(const QString &query) const;

private:
    void setEngine(QueryEngine*);
    QueryEngine *query_engine;
    friend class ::App;
};

}
