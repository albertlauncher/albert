// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/queryhandler.h"
#include "albert/logging.h"
#include "itemindex.h"
#include "queryengine.h"
#include "settings/triggerwidget.h"
using namespace albert;
using namespace std;
static const char *CFG_TRIGGER = "trigger";
static const char *CFG_TRIGGER_ENABLED = "trigger_enabled";
//static const char* CFG_ERROR_TOLERANCE_DIVISOR = "error_tolerance_divisor";
static const uint DEF_ERROR_TOLERANCE_DIVISOR = 4;
//static const char* CFG_CASE_SENSITIVE = "case_sensitive";
//static const bool  DEF_CASE_SENSITIVE = false;
static const char* CFG_FUZZY = "fuzzy";
static const bool  DEF_FUZZY = false;
static const char* CFG_SEPARATORS = "separators";
static const char* DEF_SEPARATORS = R"R([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)R";
static const uint GRAM_SIZE = 2;


QueryEngine::QueryEngine(ExtensionRegistry &registry):
        ExtensionWatcher<QueryHandler>(registry),
        ExtensionWatcher<IndexQueryHandler>(registry),
        global_search_handler_(registry)
{

    QSettings s;
    fuzzy_ = s.value(CFG_FUZZY, DEF_FUZZY).toBool();
    separators_ = s.value(CFG_SEPARATORS, DEF_SEPARATORS).toString();

    for (auto &[id, handler] : registry.extensions<QueryHandler>()) {
        query_handlers_.insert(handler);
        HandlerConfig config{
                handler->settings()->value(CFG_TRIGGER, handler->default_trigger()).toString(),
                handler->settings()->value(CFG_TRIGGER_ENABLED, true).toBool()
        };
        query_handler_configs_.emplace(handler, config);
    }
    updateActiveTriggers();
}

std::unique_ptr<albert::Query> QueryEngine::query(const QString &query_string)
{
    unique_ptr<::Query> query;

    for (const auto &[trigger, handler] : active_triggers_)
        if (query_string.startsWith(trigger))
            query = make_unique<::Query>(query_handlers_, *handler, query_string.mid(trigger.size()), trigger);

    if (!query)
        query = make_unique<::Query>(query_handlers_, global_search_handler_, query_string);

    return query;
}

void QueryEngine::updateActiveTriggers()
{
    active_triggers_.clear();

    for (const auto &[handler, config]: query_handler_configs_)
        if (config.enabled)
            if (const auto &[it, success] = active_triggers_.emplace(config.trigger, handler); !success)
                WARN << QString("Trigger conflict '%1': Already reserved for %2.").arg(config.trigger, it->second->id());


}

void QueryEngine::onAdd(QueryHandler *handler)
{
    query_handlers_.insert(handler);
    HandlerConfig conf {
        handler->settings()->value(CFG_TRIGGER, handler->default_trigger()).toString(),
        handler->settings()->value(CFG_TRIGGER_ENABLED, true).toBool()
    };
    query_handler_configs_.emplace(handler, conf);
    updateActiveTriggers();
    emit handlersChanged();
}

void QueryEngine::onRem(QueryHandler *handler)
{
    query_handlers_.erase(handler);
    query_handler_configs_.erase(handler);
    updateActiveTriggers();
    emit handlersChanged();
}

void QueryEngine::onAdd(IndexQueryHandler *handler)
{
    index_query_handlers_.insert(handler);
    handler->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE, fuzzy_?DEF_ERROR_TOLERANCE_DIVISOR:0));
}

void QueryEngine::onRem(IndexQueryHandler *handler)
{
    index_query_handlers_.erase(handler);
}


const std::map<QueryHandler*,QueryEngine::HandlerConfig> &QueryEngine::handlerConfig() const
{
    return query_handler_configs_;
}

const std::map<QString, QueryHandler *> &QueryEngine::activeTriggers() const
{
    return active_triggers_;
}

void QueryEngine::setEnabled(QueryHandler *handler, bool enabled)
{
    query_handler_configs_.at(handler).enabled = enabled;
    updateActiveTriggers();
    handler->settings()->setValue(CFG_TRIGGER_ENABLED, enabled);
    DEBG << handler->id() << (enabled ? "enabled":"disabled");
}

void QueryEngine::setTrigger(QueryHandler *handler, const QString& trigger)
{
    if (!handler->allow_trigger_remap())
        return;

    query_handler_configs_.at(handler).trigger = trigger;
    updateActiveTriggers();
    handler->settings()->setValue(CFG_TRIGGER, trigger);
    DEBG << handler->id() << " set trigger" << trigger;
}

bool QueryEngine::fuzzy() const
{
    return fuzzy_;
}

void QueryEngine::setFuzzy(bool fuzzy)
{
    fuzzy_ = fuzzy;
    QSettings().setValue(CFG_FUZZY, fuzzy);
    for (auto &iqh : index_query_handlers_)
        iqh->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE, fuzzy_?DEF_ERROR_TOLERANCE_DIVISOR:0));
}

const QString &QueryEngine::separators() const
{
    return separators_;
}

void QueryEngine::setSeparators(const QString &separators)
{
    separators_ = separators;
    QSettings().setValue(CFG_SEPARATORS, separators);
    for (auto &iqh : index_query_handlers_)
        iqh->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE, fuzzy_?DEF_ERROR_TOLERANCE_DIVISOR:0));
}
