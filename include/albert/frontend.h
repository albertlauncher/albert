// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include "export.h"
#include "plugin.h"
#include <QAbstractItemModel>
#include <QString>
#include <QWidget>

namespace albert {
/// The interface for albert frontends
/// @note This is a QObject: Must be the first class inherited, no multiple inheritance
class ALBERT_EXPORT Frontend : public QObject
{
    Q_OBJECT

public:

    virtual bool isVisible() = 0;
    virtual void setVisible(bool visible = true) = 0;

    virtual QString input() = 0;
    virtual void setInput(const QString&) = 0;

    virtual void setModel(QAbstractItemModel *) = 0;

    virtual QWidget *widget(QWidget *parent) = 0;

    void toggleVisibility() { setVisible(!isVisible()); }

signals:

    void widgetShown();
    void widgetHidden();
    void inputChanged(QString qry);

};
}
