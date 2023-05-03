// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/queryhandler.h"
#include "extensions/globalqueryhandlerprivate.h"
#include "extensions/indexqueryhandlerprivate.h"
#include "albert/logging.h"
#include "itemindex.h"
#include "query.h"
#include "queryengine.h"
#include "usagedatabase.h"
#include <QCoreApplication>
#include <QSettings>
#include <QTimer>
#include <cmath>
using namespace albert;
using namespace std;
static const char *CFG_MEMORY_DECAY = "memoryDecay";
static const double DEF_MEMORY_DECAY = 0.5;
static const char *CFG_PRIO_PERFECT = "prioritize_perfect_match";
static const bool DEF_PRIO_PERFECT = true;
static const char *CFG_TRIGGER = "trigger";
static const char *CFG_TRIGGER_ENABLED = "trigger_enabled";
static const uint DEF_ERROR_TOLERANCE_DIVISOR = 4;
static const char* CFG_FUZZY = "fuzzy";
static const bool  DEF_FUZZY = false;
static const char* CFG_SEPARATORS = "separators";
static const char* DEF_SEPARATORS = R"R([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)R";
static const uint GRAM_SIZE = 2;

QueryEngine::QueryEngine(ExtensionRegistry &registry):
    ExtensionWatcher<TriggerQueryHandler>(registry),
    ExtensionWatcher<GlobalQueryHandler>(registry),
    ExtensionWatcher<IndexQueryHandler>(registry),
    ExtensionWatcher<FallbackHandler>(registry)
{
    UsageDatabase::initializeDatabase();

    QSettings s(qApp->applicationName());
    fuzzy_ = s.value(CFG_FUZZY, DEF_FUZZY).toBool();
    separators_ = s.value(CFG_SEPARATORS, DEF_SEPARATORS).toString();
    memory_decay_ = s.value(CFG_MEMORY_DECAY, DEF_MEMORY_DECAY).toDouble();
    prioritize_perfect_match_ = s.value(CFG_PRIO_PERFECT, DEF_PRIO_PERFECT).toBool();

    updateUsageScore();
    GlobalQueryHandlerPrivate::setPrioritizePerfectMatch(prioritize_perfect_match_);

    for (auto &[id, handler] : registry.extensions<QueryHandler>()) {
        trigger_query_handlers_.insert(handler);
        HandlerConfig config{
                handler->settings()->value(CFG_TRIGGER, handler->defaultTrigger()).toString(),
                handler->settings()->value(CFG_TRIGGER_ENABLED, true).toBool()
        };
        query_handler_configs_.emplace(handler, config);
    }
    updateActiveTriggers();
}

shared_ptr<albert::Query> QueryEngine::query(const QString &query_string)
{
    shared_ptr<::Query> query;

    for (const auto &[trigger, handler] : active_triggers_)
        if (query_string.startsWith(trigger))
            query = make_shared<::Query>(fallback_handlers_, handler, query_string.mid(trigger.size()), trigger);

    if (!query)
        query = make_shared<::Query>(fallback_handlers_, &global_search_handler, query_string);

    auto onActivate = [this, q=query->string()](const QString& e, const QString &i, const QString &a){
        UsageDatabase::addActivation(q, e, i, a);
        QTimer::singleShot(0, [this](){ updateUsageScore(); });
    };

    QObject::connect(&query->matches_, &ItemsModel::activated, onActivate);
    QObject::connect(&query->fallbacks_, &ItemsModel::activated, onActivate);  // TODO differentiate

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

void QueryEngine::onAdd(TriggerQueryHandler *handler)
{
    trigger_query_handlers_.insert(handler);
    HandlerConfig conf {
        handler->settings()->value(CFG_TRIGGER, handler->defaultTrigger()).toString(),
        handler->settings()->value(CFG_TRIGGER_ENABLED, true).toBool()
    };
    query_handler_configs_.emplace(handler, conf);
    updateActiveTriggers();
}

void QueryEngine::onRem(TriggerQueryHandler *handler)
{
    trigger_query_handlers_.erase(handler);
    query_handler_configs_.erase(handler);
    updateActiveTriggers();
}

void QueryEngine::onAdd(GlobalQueryHandler *handler)
{
    global_search_handler.handlers.insert(handler->d.get());
}

void QueryEngine::onRem(GlobalQueryHandler *handler)
{
    global_search_handler.handlers.erase(handler->d.get());
}

void QueryEngine::onAdd(IndexQueryHandler *handler)
{
    index_query_handlers_.insert(handler);
    handler->d->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE, fuzzy_?DEF_ERROR_TOLERANCE_DIVISOR:0));
}

void QueryEngine::onRem(IndexQueryHandler *handler)
{
    index_query_handlers_.erase(handler);
}

void QueryEngine::onAdd(FallbackHandler *handler)
{
    fallback_handlers_.insert(handler);
}

void QueryEngine::onRem(FallbackHandler *handler)
{
    fallback_handlers_.erase(handler);
}

const map<TriggerQueryHandler*,QueryEngine::HandlerConfig> &QueryEngine::handlerConfig() const
{
    return query_handler_configs_;
}

const map<QString, TriggerQueryHandler *> &QueryEngine::activeTriggers() const
{
    return active_triggers_;
}

void QueryEngine::setEnabled(TriggerQueryHandler *handler, bool enabled)
{
    query_handler_configs_.at(handler).enabled = enabled;
    updateActiveTriggers();
    handler->settings()->setValue(CFG_TRIGGER_ENABLED, enabled);
}

void QueryEngine::setTrigger(TriggerQueryHandler *handler, const QString& trigger)
{
    if (!handler->allowTriggerRemap())
        return;

    query_handler_configs_.at(handler).trigger = trigger;
    updateActiveTriggers();
    handler->settings()->setValue(CFG_TRIGGER, trigger);
}

void QueryEngine::updateUsageScore() const
{
    vector<Activation> activations = UsageDatabase::activations();

    // Compute usage weights
    map<pair<QString,QString>,double> usage_weights;
    for (int i = 0, k = (int)activations.size(); i < (int)activations.size(); ++i, --k){
        auto activation = activations[i];
        auto weight = pow(memory_decay_, k);
        if (const auto &[it, success] =
            usage_weights.emplace(make_pair(activation.extension_id, activation.item_id), weight);
            !success)
            it->second += weight;
    }

    // Invert the list. Results in ordered by rank map
    map<double,vector<pair<QString,QString>>> weight_items;
    for (const auto &[ids, weight] : usage_weights)
        weight_items[weight].emplace_back(ids);

    // Distribute scores linearly over the interval preserving the order
    map<pair<QString,QString>,RankItem::Score> usage_scores;
    double rank = 0.0;
    for (const auto &[weight, vids] : weight_items){
        RankItem::Score score = rank / (double)weight_items.size() * RankItem::MAX_SCORE;
        for (const auto &ids : vids)
            usage_scores.emplace(ids, score);
        rank += 1.0;
    }

    GlobalQueryHandlerPrivate::setScores(usage_scores);
}

double QueryEngine::memoryDecay() const
{
    return memory_decay_;
}

// @param forgetfulness: must be in range [0.5,1] i.e. from [MRU,MFU]
void QueryEngine::setMemoryDecay(double val)
{
    memory_decay_ = val;
    QSettings(qApp->applicationName()).setValue(CFG_MEMORY_DECAY, val);
    updateUsageScore();
}

bool QueryEngine::prioritizePerfectMatch() const
{
    return prioritize_perfect_match_;
}

void QueryEngine::setPrioritizePerfectMatch(bool val)
{
    prioritize_perfect_match_ = val;
    QSettings(qApp->applicationName()).setValue(CFG_PRIO_PERFECT, val);
    GlobalQueryHandlerPrivate::setPrioritizePerfectMatch(prioritize_perfect_match_);
}


bool QueryEngine::fuzzy() const
{
    return fuzzy_;
}

void QueryEngine::setFuzzy(bool fuzzy)
{
    fuzzy_ = fuzzy;
    QSettings(qApp->applicationName()).setValue(CFG_FUZZY, fuzzy);
    for (auto &iqh : index_query_handlers_)
        iqh->d->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE, fuzzy_ ? DEF_ERROR_TOLERANCE_DIVISOR : 0));
}

const QString &QueryEngine::separators() const
{
    return separators_;
}

void QueryEngine::setSeparators(const QString &separators)
{
    separators_ = separators;
    QSettings(qApp->applicationName()).setValue(CFG_SEPARATORS, separators);
    for (auto &iqh : index_query_handlers_)
        iqh->d->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE, fuzzy_ ? DEF_ERROR_TOLERANCE_DIVISOR : 0));
}
