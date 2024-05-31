// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "indexqueryhandler.h"
#include <QCoreApplication>
class PluginRegistry;

class PluginQueryHandler : public albert::IndexQueryHandler
{
    Q_DECLARE_TR_FUNCTIONS(PluginQueryHandler)

public:
    PluginQueryHandler(PluginRegistry &plugin_registry);

    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void updateIndexItems() override;

private:
    PluginRegistry &plugin_registry_;
};
