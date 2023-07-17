// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/queryhandler.h"
#include <QObject>
namespace albert { class ExtensionRegistry; }

class AppQueryHandler : public albert::QueryHandler
{
public:
    AppQueryHandler(albert::ExtensionRegistry *);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;

private:
    albert::ExtensionRegistry *registry_;
};

