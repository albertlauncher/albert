// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include <QObject>
namespace albert { class ExtensionRegistry; }

class AppQueryHandler : public albert::GlobalQueryHandler
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
    std::vector<std::shared_ptr<albert::Item>> items_;
    static const QStringList icon_urls;
};

