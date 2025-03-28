// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "globalqueryhandler.h"
#include <QCoreApplication>

class AppQueryHandler : public albert::GlobalQueryHandler
{
    Q_DECLARE_TR_FUNCTIONS(AppQueryHandler)

public:
    AppQueryHandler();
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;

private:

    static const QStringList icon_urls;
    struct {
        const QString cache;
        const QString cached;
        const QString config;
        const QString configd;
        const QString data;
        const QString datad;
        const QString open;
        const QString topen;
        const QString quit;
        const QString quitd;
        const QString restart;
        const QString restartd;
        const QString settings;
        const QString settingsd;
    } strings;
};
