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
#include <QMessageBox>
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
static const char *CFG_TRIGGER_HANDLER_ENABLED = "trigger_handler_enabled";
static const char *CFG_GLOBAL_HANDLER_ENABLED = "global_handler_enabled";
static const char *CFG_FALLBACK_HANDLER_ENABLED = "fallback_hanlder_enabled";
static const uint DEF_ERROR_TOLERANCE_DIVISOR = 4;
static const char* CFG_FUZZY = "fuzzy";
static const bool  DEF_FUZZY = false;
static const char* CFG_SEPARATORS = "separators";
static const char* DEF_SEPARATORS = R"R([\s\\\/\-\[\](){}#!?<>"'=+*.:,;_]+)R";
static const uint GRAM_SIZE = 2;

QueryEngine::QueryEngine(ExtensionRegistry &registry):
    ExtensionWatcher<TriggerQueryHandler>(registry),
    ExtensionWatcher<GlobalQueryHandler>(registry),
    ExtensionWatcher<FallbackHandler>(registry),
    global_search_handler(enabled_global_handlers_)
{
    UsageDatabase::initializeDatabase();

    QSettings s(qApp->applicationName());
    fuzzy_ = s.value(CFG_FUZZY, DEF_FUZZY).toBool();
    separators_ = s.value(CFG_SEPARATORS, DEF_SEPARATORS).toString();
    memory_decay_ = s.value(CFG_MEMORY_DECAY, DEF_MEMORY_DECAY).toDouble();
    prioritize_perfect_match_ = s.value(CFG_PRIO_PERFECT, DEF_PRIO_PERFECT).toBool();

    updateUsageScore();
    GlobalQueryHandlerPrivate::setPrioritizePerfectMatch(prioritize_perfect_match_);
}

shared_ptr<albert::Query> QueryEngine::query(const QString &query_string)
{
    shared_ptr<::Query> query;

    for (const auto &[handler, trigger] : enabled_trigger_handlers_)
        if (query_string.startsWith(trigger))
            query = make_shared<::Query>(enabled_fallback_handlers_, handler, query_string.mid(trigger.size()), trigger);

    if (!query)
        query = make_shared<::Query>(enabled_fallback_handlers_, &global_search_handler, query_string);

    auto onActivate = [this, q=query->string()](const QString& e, const QString &i, const QString &a){
        UsageDatabase::addActivation(q, e, i, a);
        QTimer::singleShot(0, [this](){ updateUsageScore(); });
    };

    QObject::connect(&query->matches_, &ItemsModel::activated, onActivate);
    QObject::connect(&query->fallbacks_, &ItemsModel::activated, onActivate);  // TODO differentiate

    return query;
}

const map<albert::TriggerQueryHandler*, QString> &QueryEngine::triggerHandlers()
{ return trigger_handlers_; }

const set<GlobalQueryHandler*> &QueryEngine::globalHandlers()
{ return global_handlers_; }

const set<FallbackHandler*> &QueryEngine::fallbackHandlers()
{ return fallback_handlers_; }

bool QueryEngine::isEnabled(albert::TriggerQueryHandler *handler) const
{ return enabled_trigger_handlers_.contains(handler); }

bool QueryEngine::isEnabled(albert::GlobalQueryHandler *handler) const
{ return enabled_global_handlers_.contains(handler); }

bool QueryEngine::isEnabled(albert::FallbackHandler *handler) const
{ return enabled_fallback_handlers_.contains(handler); }

const QString &QueryEngine::trigger(albert::TriggerQueryHandler *handler) const
{ return trigger_handlers_.at(handler); }

bool QueryEngine::setTrigger(TriggerQueryHandler *handler, const QString& trigger)
{
    if (!handler->allowTriggerRemap())
        return false;

    setEnabled(handler, false);

    if (trigger.isEmpty()){
        trigger_handlers_.at(handler) = handler->defaultTrigger();
        handler->settings()->remove(CFG_TRIGGER);
    } else {
        trigger_handlers_.at(handler) = trigger;
        handler->settings()->setValue(CFG_TRIGGER, trigger);
    }

    return setEnabled(handler);
}

bool QueryEngine::setEnabled(albert::TriggerQueryHandler *handler, bool enabled)
{
    if (isEnabled(handler) == enabled)
        return true;

    auto trigger = trigger_handlers_.at(handler);

    if (enabled){
        // Check for conflicts
        auto trigger_conflict = std::find_if(
            enabled_trigger_handlers_.begin(), enabled_trigger_handlers_.end(),
            [trigger](const auto& kv) {return kv.second == trigger; }) != enabled_trigger_handlers_.end();

        if (trigger_conflict){
            WARN << QString("Trigger conflict: '%1' reserved by '%2'.").arg(trigger, handler->id());
            return false;
        } else {
            handler->settings()->remove(CFG_TRIGGER_HANDLER_ENABLED);
            enabled_trigger_handlers_.emplace(handler, trigger_handlers_.at(handler));
            return true;
        }
    } else {
        enabled_trigger_handlers_.erase(handler);
        handler->settings()->setValue(CFG_TRIGGER_HANDLER_ENABLED, false);
        return true;
    }
}

void QueryEngine::setEnabled(GlobalQueryHandler *handler, bool enabled)
{
    if (enabled){
        enabled_global_handlers_.emplace(handler);
        if (auto *ih = dynamic_cast<IndexQueryHandler*>(handler); ih)
            ih->d->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE,
                                                   fuzzy_?DEF_ERROR_TOLERANCE_DIVISOR:0));
    } else {
        enabled_global_handlers_.erase(handler);
        if (auto *ih = dynamic_cast<IndexQueryHandler*>(handler); ih)
            ih->d->setIndex(nullptr);
    }
    handler->settings()->setValue(CFG_GLOBAL_HANDLER_ENABLED, enabled);
}

void QueryEngine::setEnabled(FallbackHandler *handler, bool enabled)
{
    if (enabled)
        enabled_fallback_handlers_.emplace(handler);
    else
        enabled_fallback_handlers_.erase(handler);
    handler->settings()->setValue(CFG_FALLBACK_HANDLER_ENABLED, enabled);
}

void QueryEngine::onAdd(TriggerQueryHandler *handler)
{
    trigger_handlers_.emplace(handler, handler->allowTriggerRemap()
                                           ? handler->settings()->value(CFG_TRIGGER, handler->defaultTrigger()).toString()
                                           : handler->defaultTrigger());

    if (handler->settings()->value(CFG_TRIGGER_HANDLER_ENABLED, true).toBool())
        if(!setEnabled(handler))
            WARN << QString("Failed activating trigger '%1'").arg(handler->id());
}

void QueryEngine::onRem(TriggerQueryHandler *handler)
{
    enabled_trigger_handlers_.erase(handler);
    trigger_handlers_.erase(handler);
}

void QueryEngine::onAdd(GlobalQueryHandler *handler)
{
    global_handlers_.emplace(handler);
    if (handler->settings()->value(CFG_GLOBAL_HANDLER_ENABLED, true).toBool())
        setEnabled(handler);
}

void QueryEngine::onRem(GlobalQueryHandler *handler)
{
    global_handlers_.erase(handler);
    enabled_global_handlers_.erase(handler);
}

void QueryEngine::onAdd(FallbackHandler *handler)
{
    fallback_handlers_.emplace(handler);
    if (handler->settings()->value(CFG_FALLBACK_HANDLER_ENABLED, true).toBool())
        setEnabled(handler);
}

void QueryEngine::onRem(FallbackHandler *handler)
{
    fallback_handlers_.erase(handler);
    enabled_fallback_handlers_.erase(handler);
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

double QueryEngine::memoryDecay() const { return memory_decay_; }

// @param forgetfulness: must be in range [0.5,1] i.e. from [MRU,MFU]
void QueryEngine::setMemoryDecay(double val)
{
    memory_decay_ = val;
    QSettings(qApp->applicationName()).setValue(CFG_MEMORY_DECAY, val);
    updateUsageScore();
}

bool QueryEngine::prioritizePerfectMatch() const
{ return prioritize_perfect_match_; }

void QueryEngine::setPrioritizePerfectMatch(bool val)
{
    prioritize_perfect_match_ = val;
    QSettings(qApp->applicationName()).setValue(CFG_PRIO_PERFECT, val);
    GlobalQueryHandlerPrivate::setPrioritizePerfectMatch(prioritize_perfect_match_);
}

bool QueryEngine::fuzzy() const { return fuzzy_; }

void QueryEngine::setFuzzy(bool fuzzy)
{
    fuzzy_ = fuzzy;
    QSettings(qApp->applicationName()).setValue(CFG_FUZZY, fuzzy);
    for (auto *handler : enabled_global_handlers_)
        if (auto *ih = dynamic_cast<IndexQueryHandler*>(handler); ih)
            ih->d->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE,
                                                   fuzzy_ ? DEF_ERROR_TOLERANCE_DIVISOR : 0));
}

const QString &QueryEngine::separators() const { return separators_; }

void QueryEngine::setSeparators(const QString &separators)
{
    separators_ = separators;
    QSettings(qApp->applicationName()).setValue(CFG_SEPARATORS, separators);
    for (auto *handler : enabled_global_handlers_)
        if (auto *ih = dynamic_cast<IndexQueryHandler*>(handler); ih)
            ih->d->setIndex(make_unique<ItemIndex>(separators_, false, GRAM_SIZE,
                                                   fuzzy_ ? DEF_ERROR_TOLERANCE_DIVISOR : 0));
}
