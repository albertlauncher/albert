// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include "plugin.h"
#include "core_globals.h"

class QAbstractItemModel;

#define ALBERT_FRONTEND_IID ALBERT_PLUGIN_IID_PREFIX".frontendv1-alpha"

namespace Core {

class EXPORT_CORE Frontend : public Plugin
{
    Q_OBJECT

public:

    Frontend(const QString &id) : Plugin(id) {}

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
    void settingsWidgetRequested();

};

}
