// Copyright (c) 2024 Manuel Schneider

#pragma once
#include "albert/query/globalqueryhandler.h"
#include <QCoreApplication>
class PluginRegistry;

class PluginConfigQueryHandler : public albert::GlobalQueryHandler
{
    Q_DECLARE_TR_FUNCTIONS(PluginConfigQueryHandler)

public:
    PluginConfigQueryHandler(PluginRegistry&);

    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query*) const override;

private:
    PluginRegistry &plugin_registry_;
    static const QStringList icon_urls;

};

