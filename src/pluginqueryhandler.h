// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
class PluginRegistry;

class PluginQueryHandler : public albert::IndexQueryHandler
{
    PluginRegistry &plugin_registry_;
public:
    PluginQueryHandler(PluginRegistry &plugin_registry);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void updateIndexItems() override;
};
