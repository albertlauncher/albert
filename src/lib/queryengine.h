// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/indexqueryhandler.h"
#include "albert/extensions/queryhandler.h"
#include "albert/util/extensionwatcher.h"
#include "globalsearch.h"
#include "query.h"
#include <QObject>
#include <map>
#include <memory>
#include <set>

class QueryEngine:
        public QObject,
        public albert::ExtensionWatcher<albert::QueryHandler>,
        public albert::ExtensionWatcher<albert::IndexQueryHandler>

{
    Q_OBJECT
public:
    struct HandlerConfig {
        QString trigger;
        bool enabled;
    };

    explicit QueryEngine(albert::ExtensionRegistry&);

    const std::map<albert::QueryHandler*,HandlerConfig> &handlerConfig() const;
    void setTrigger(albert::QueryHandler*, const QString&);
    void setEnabled(albert::QueryHandler*, bool);
    const std::map<QString,albert::QueryHandler*> &activeTriggers() const;

    bool fuzzy() const;
    void setFuzzy(bool);

    const QString &separators() const;
    void setSeparators(const QString &);

    std::unique_ptr<albert::Query> query(const QString &query);

private:
    void updateActiveTriggers();
    void onAdd(albert::QueryHandler*) override;
    void onRem(albert::QueryHandler*) override;
    void onAdd(albert::IndexQueryHandler*) override;
    void onRem(albert::IndexQueryHandler*) override;

    GlobalSearch global_search_handler_;
    std::set<albert::QueryHandler*> query_handlers_;
    std::set<albert::IndexQueryHandler*> index_query_handlers_;


    std::map<albert::QueryHandler*,HandlerConfig> query_handler_configs_;
    std::map<QString,albert::QueryHandler*> active_triggers_;
    bool fuzzy_;
    QString separators_;
signals:
    void handlersChanged();
};
