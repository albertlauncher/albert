// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>
#include <QString>
class QWidget;

namespace albert
{

class Query;

///
/// The interface for albert frontends.
///
class ALBERT_EXPORT Frontend : public QObject
{
    Q_OBJECT

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

    /// The query setter
    virtual void setQuery(Query *query) = 0;

signals:

    void inputChanged(QString);
    void visibleChanged(bool);

};

}
