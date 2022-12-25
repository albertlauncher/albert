// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/queryhandler.h"
#include "albert/logging.h"
#include "itemindex.h"
#include "usagedatabase.h"
#include "queryengine.h"
#include "query.h"
#include "settings/triggerwidget.h"
#include <QTimer>
#include <cmath>
using namespace albert;
using namespace std;
static const char *CFG_MEMORY_DECAY = "memoryDecay";
static const double DEF_MEMORY_DECAY = 0.75;
static const char *CFG_MEMORY_WEIGHT = "memoryWeight";
static const double DEF_MEMORY_WEIGHT = 0.5;
static const char *CFG_TRIGGER = "trigger";
static const char *CFG_TRIGGER_ENABLED = "trigger_enabled";
static const uint DEF_ERROR_TOLERANCE_DIVISOR = 4;
static const char* CFG_FUZZY = "fuzzy";
static const bool  DEF_FUZZY = false;
static const char* CFG_SEPARATORS = "separators";
static const char* DEF_SEPARATORS = R"R([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)R";
static const uint GRAM_SIZE = 2;

QueryEngine::QueryEngine(ExtensionRegistry &registry):
        ExtensionWatcher<QueryHandler>(registry),
        ExtensionWatcher<GlobalQueryHandler>(registry),
        ExtensionWatcher<IndexQueryHandler>(registry)
{

    QSettings s;
    fuzzy_ = s.value(CFG_FUZZY, DEF_FUZZY).toBool();
    separators_ = s.value(CFG_SEPARATORS, DEF_SEPARATORS).toString();
    memory_decay_ = s.value(CFG_MEMORY_DECAY, DEF_MEMORY_DECAY).toDouble();
    memory_weight_ = s.value(CFG_MEMORY_WEIGHT, DEF_MEMORY_WEIGHT).toDouble();

    UsageDatabase::initializeDatabase();
    updateUsageScore();

    for (auto &[id, handler] : registry.extensions<QueryHandler>()) {
        query_handlers_.insert(handler);
        HandlerConfig config{
                handler->settings()->value(CFG_TRIGGER, handler->defaultTrigger()).toString(),
                handler->settings()->value(CFG_TRIGGER_ENABLED, true).toBool()
        };
        query_handler_configs_.emplace(handler, config);
    }
    updateActiveTriggers();
}

std::shared_ptr<albert::Query> QueryEngine::query(const QString &query_string)
{
    shared_ptr<::Query> query;

    for (const auto &[trigger, handler] : active_triggers_)
        if (query_string.startsWith(trigger))
            query = make_shared<::Query>(query_handlers_, handler, query_string.mid(trigger.size()), trigger);

    if (!query)
        query = make_shared<::Query>(query_handlers_, &global_search_handler, query_string);

    QObject::connect(query.get(), &::Query::activated,
                     [this, query=query->string()](const QString& e, const QString &i, const QString &a){
        UsageDatabase::addActivation(query, e, i, a);
        QTimer::singleShot(0,[this](){updateUsageScore();});
    });
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
        handler->settings()->value(CFG_TRIGGER, handler->defaultTrigger()).toString(),
        handler->settings()->value(CFG_TRIGGER_ENABLED, true).toBool()
    };
    query_handler_configs_.emplace(handler, conf);
    updateActiveTriggers();
}

void QueryEngine::onRem(QueryHandler *handler)
{
    query_handlers_.erase(handler);
    query_handler_configs_.erase(handler);
    updateActiveTriggers();
}

void QueryEngine::onAdd(GlobalQueryHandler *handler)
{
    global_search_handler.handlers.insert(handler);
}

void QueryEngine::onRem(GlobalQueryHandler *handler)
{
    global_search_handler.handlers.erase(handler);
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
}

void QueryEngine::setTrigger(QueryHandler *handler, const QString& trigger)
{
    if (!handler->allowTriggerRemap())
        return;

    query_handler_configs_.at(handler).trigger = trigger;
    updateActiveTriggers();
    handler->settings()->setValue(CFG_TRIGGER, trigger);
}

void QueryEngine::updateUsageScore() const
{
    std::map<std::pair<QString,QString>,double> usage_scores;
    std::vector<Activation> activations = UsageDatabase::activations();
    double max_score = 0;
    for (int i = 0, k = (int)activations.size(); i < (int)activations.size(); ++i, --k){
        auto activation = activations[i];
        auto memory_weight = pow(memory_decay_, k);

        if (const auto &[it, success] = usage_scores.emplace(make_pair(activation.extension_id,
                                                                       activation.item_id),
                                                             memory_weight); !success){
            it->second += memory_weight;
            if (max_score < it->second)
                max_score = it->second;
        } else
        if (max_score < memory_weight)
            max_score = memory_weight;
    }

    // Normalize
    for (auto &[ids, score] : usage_scores)
        score = score/max_score;

    GlobalQueryHandler::setScores(usage_scores);
}

double QueryEngine::memoryDecay() const
{
    return memory_decay_;
}

// @param forgetfulness: must be in range [0.5,1] i.e. from [MRU,MFU]
void QueryEngine::setMemoryDecay(double val)
{
    memory_decay_ = val;
    QSettings().setValue(CFG_MEMORY_DECAY, val);
    updateUsageScore();
}

double QueryEngine::memoryWeight() const
{
    return memory_weight_;
}

void QueryEngine::setMemoryWeight(double val)
{
    memory_weight_ = val;
    QSettings().setValue(CFG_MEMORY_WEIGHT, val);
    GlobalQueryHandler::setWeight(memory_weight_);
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