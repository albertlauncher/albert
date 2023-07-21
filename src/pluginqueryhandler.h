// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
class PluginRegistry;

class PluginQueryHandler : public albert::TriggerQueryHandler
{
    PluginRegistry &plugin_registry_;
public:
    PluginQueryHandler(PluginRegistry &plugin_registry);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};
