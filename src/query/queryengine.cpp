// Copyright (c) 2022-2025 Manuel Schneider

#include "albert.h"
#include "extensionregistry.h"
#include "fallbackhandler.h"
#include "globalqueryhandler.h"
#include "logging.h"
#include "queryengine.h"
#include "queryexecution.h"
#include "triggerqueryhandler.h"
#include "usagedatabase.h"
#include "usagescoring.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
using namespace albert;
using namespace std;

namespace
{
static const char*  CFG_GLOBAL_HANDLER_ENABLED = "global_handler_enabled";
static const char*  CFG_FALLBACK_ORDER = "fallback_order";
static const char*  CFG_FALLBACK_EXTENSION = "extension";
static const char*  CFG_FALLBACK_ITEM = "fallback";
static const char*  CFG_TRIGGER = "trigger";
static const char*  CFG_FUZZY = "fuzzy";
static const char*  CFG_MEMORY_DECAY = "memoryDecay";
static const double DEF_MEMORY_DECAY = 0.5;
static const char*  CFG_PRIO_PERFECT = "prioritizePerfectMatch";
static const bool   DEF_PRIO_PERFECT = true;
}

QueryEngine::QueryEngine(ExtensionRegistry &registry):
    registry_(registry)
{
    auto s = settings();
    auto decay = s->value(CFG_MEMORY_DECAY, DEF_MEMORY_DECAY).toDouble();
    auto prioritize_perfect_match = s->value(CFG_PRIO_PERFECT, DEF_PRIO_PERFECT).toBool();
    usage_scoring_ = UsageScoring(prioritize_perfect_match, decay,
                                  make_shared<unordered_map<ItemKey, double>>
                                  (UsageDatabase::instance().itemUsageScores(decay)));

    loadFallbackOrder();

    connect(&registry, &ExtensionRegistry::added, this, [this](Extension *e) {
        if (auto *th = dynamic_cast<albert::TriggerQueryHandler*>(e))
        {
            auto sett = settings();
            sett->beginGroup(th->id());
            auto t = sett->value(CFG_TRIGGER, th->defaultTrigger()).toString();
            auto f = sett->value(CFG_FUZZY, false).toBool();

            th->setTrigger(t);
            th->setFuzzyMatching(f);
            trigger_handlers_.emplace(piecewise_construct,
                                      forward_as_tuple(th->id()),
                                      forward_as_tuple(th, t, f));
            updateActiveTriggers();

            if (auto *gh = dynamic_cast<albert::GlobalQueryHandler*>(th))
            {
                auto en = settings()->value(QString("%1/%2")
                                                .arg(gh->id(), CFG_GLOBAL_HANDLER_ENABLED),
                                            true).toBool();
                global_handlers_.emplace(piecewise_construct,
                                         forward_as_tuple(gh->id()),
                                         forward_as_tuple(gh, en));
            }

            emit handlerAdded();
        }
        if (auto *fh = dynamic_cast<albert::FallbackHandler*>(e))
        {
            fallback_handlers_.emplace(fh->id(), fh);
            emit handlerAdded();
        }
    });

    connect(&registry, &ExtensionRegistry::removed, this, [this](Extension *e) {
        if (auto *th = dynamic_cast<albert::TriggerQueryHandler*>(e))
        {
            trigger_handlers_.erase(th->id());
            updateActiveTriggers();

            if (auto *gh = dynamic_cast<albert::GlobalQueryHandler*>(th))
                global_handlers_.erase(gh->id());

            emit handlerRemoved();
        }
        if (auto *fh = dynamic_cast<albert::FallbackHandler*>(e))
        {
            fallback_handlers_.erase(fh->id());
            emit handlerRemoved();
        }
    });
}

void QueryEngine::setMemoryDecay(double v)
{
    lock_guard lock(usage_scoring_mutex_);
    if (usage_scoring_.memory_decay != v)
    {
        DEBG << "memoryDecay set to" << v;
        settings()->setValue(CFG_MEMORY_DECAY, v);
        usage_scoring_ = UsageScoring(usage_scoring_.prioritize_perfect_match, v,
                                      make_shared<unordered_map<ItemKey, double>>
                                      (UsageDatabase::instance().itemUsageScores(v)));
    }
}

void QueryEngine::setPrioritizePerfectMatch(bool v)
{
    lock_guard lock(usage_scoring_mutex_);
    if (usage_scoring_.prioritize_perfect_match != v)
    {
        DEBG << "prioritizePerfectMatch set to" << v;
        settings()->setValue(CFG_PRIO_PERFECT, v);
        usage_scoring_ = UsageScoring(v, usage_scoring_.memory_decay, usage_scoring_.usage_scores);
    }
}

void QueryEngine::storeItemActivation(const QString &query, const QString &extension,
                                      const QString &item, const QString &action)
{
    UsageDatabase::instance().addActivation(query, extension, item, action);

    auto scores = UsageDatabase::instance().itemUsageScores(usage_scoring_.memory_decay);

    lock_guard lock(usage_scoring_mutex_);
    usage_scoring_ = UsageScoring(
        usage_scoring_.prioritize_perfect_match,
        usage_scoring_.memory_decay,
        make_shared<unordered_map<ItemKey, double>>(::move(scores))
    );
}

UsageScoring QueryEngine::usageScoring() const
{
    lock_guard lock(usage_scoring_mutex_);
    return usage_scoring_;
}

unique_ptr<QueryExecution> QueryEngine::query(const QString &query)
{
    vector<FallbackHandler*> fhandlers;
    for (const auto&[id, handler] : fallback_handlers_)
        fhandlers.emplace_back(handler);

    for (const auto &[trigger, handler] : active_triggers_)
        if (query.startsWith(trigger))
            return make_unique<QueryExecution>(this, ::move(fhandlers), handler,
                                               query.mid(trigger.size()), trigger);

    {
        vector<albert::GlobalQueryHandler*> handlers;
        for (const auto&[id, h] : global_handlers_)
            if (h.enabled)
                handlers.emplace_back(h.handler);

        return make_unique<GlobalQuery>(this, ::move(fhandlers), ::move(handlers),
                                        query == QStringLiteral("*")
                                            ? QString("")
                                            : query.isEmpty() ? QString{} : query);
    }
}

//
// Trigger handlers
//

map<QString, TriggerQueryHandler*> QueryEngine::triggerHandlers()
{
    map<QString, albert::TriggerQueryHandler*> handlers;
    for (const auto &[id, h] : trigger_handlers_)
        handlers.emplace(id, h.handler);
    return handlers;
}

const map<QString, TriggerQueryHandler *> &QueryEngine::activeTriggerHandlers() const
{ return active_triggers_; }

void QueryEngine::updateActiveTriggers()
{
    active_triggers_.clear();
    for (const auto&[id, h] : trigger_handlers_)
        if (const auto&[it, success] = active_triggers_.emplace(h.trigger, h.handler); !success)
            WARN << QString("Trigger '%1' of '%2' already registered for '%3'.")
                        .arg(h.trigger, id, it->second->id());
    emit activeTriggersChanged();
}

QString QueryEngine::trigger(const QString &id) const
{ return trigger_handlers_.at(id).trigger; }

void QueryEngine::setTrigger(const QString &id, const QString& t)
{
    auto &h = trigger_handlers_.at(id);

    if (h.trigger == t || !h.handler->allowTriggerRemap())
        return;

    if (t.isEmpty() || t == h.handler->defaultTrigger())
    {
        h.trigger = h.handler->defaultTrigger();
        settings()->remove(QString("%1/%2").arg(id, CFG_TRIGGER));
    }
    else
    {
        h.trigger = t;
        settings()->setValue(QString("%1/%2").arg(id, CFG_TRIGGER), t);
    }

    h.handler->setTrigger(h.trigger);
    updateActiveTriggers();
}

bool QueryEngine::fuzzy(const QString &id) const
{ return trigger_handlers_.at(id).fuzzy; }

void QueryEngine::setFuzzy(const QString &id, bool f)
{
    auto &h = trigger_handlers_.at(id);

    if (h.handler->supportsFuzzyMatching())
    {
        h.fuzzy = f;
        settings()->setValue(QString("%1/%2").arg(id, CFG_FUZZY), f);
        h.handler->setFuzzyMatching(f);
    }
}


//
// Global handlers
//

map<QString, GlobalQueryHandler*> QueryEngine::globalHandlers()
{
    map<QString, albert::GlobalQueryHandler*> handlers;
    for (const auto &[id, h] : global_handlers_)
        handlers.emplace(id, h.handler);
    return handlers;
}

bool QueryEngine::isEnabled(const QString &id) const
{ return global_handlers_.at(id).enabled; }

void QueryEngine::setEnabled(const QString &id, bool e)
{
    auto &h = global_handlers_.at(id);

    if (h.enabled != e)
    {
        settings()->setValue(QString("%1/%2").arg(id, CFG_GLOBAL_HANDLER_ENABLED), e);
        h.enabled = e;
    }
}


//
// Fallback handlers
//

map<QString, FallbackHandler*> QueryEngine::fallbackHandlers()
{ return fallback_handlers_; }

map<pair<QString,QString>,int> QueryEngine::fallbackOrder() const
{ return fallback_order_; }

void QueryEngine::setFallbackOrder(map<pair<QString,QString>,int> order)
{
    fallback_order_ = order;
    saveFallbackOrder();
}

// bool QueryEngine::isEnabled(const FallbackHandler *h) const
// { return enabled_fallback_handlers_.contains(h->id()); }

// void QueryEngine::setEnabled(FallbackHandler *h, bool e)
// {
//     if (isEnabled(h) == e)
//         return;

//     settings()->setValue(QString("%1/%2").arg(h->id(), CFG_FHANDLER_ENABLED), e);

//     if (e)
//         enabled_fallback_handlers_.emplace(h->id(), h);
//     else
//         enabled_fallback_handlers_.erase(h->id());

//     emit handlersChanged();
// }

void QueryEngine::saveFallbackOrder() const
{
    // Invert to ordered list
    vector<pair<QString, QString>> o;
    for (const auto &[pair, prio] : fallback_order_)
        o.emplace_back(pair.first, pair.second);
    sort(begin(o), end(o), [&](const auto &a, const auto &b)
         { return fallback_order_.at(a) > fallback_order_.at(b); });

    // Save to settings
    auto s = settings();
    s->beginWriteArray(CFG_FALLBACK_ORDER);
    for (int i = 0; i < (int)o.size(); ++i)
    {
        s->setArrayIndex(i);
        s->setValue(CFG_FALLBACK_EXTENSION, o.at(i).first);
        s->setValue(CFG_FALLBACK_ITEM, o.at(i).second);
    }
    s->endArray();
}

void QueryEngine::loadFallbackOrder()
{
    // Load from settings
    vector<pair<QString, QString>> o;
    auto s = settings();
    int size = s->beginReadArray(CFG_FALLBACK_ORDER);
    for (int i = 0; i < size; ++i)
    {
        s->setArrayIndex(i);
        o.emplace_back(s->value(CFG_FALLBACK_EXTENSION).toString(),
                       s->value(CFG_FALLBACK_ITEM).toString());
    }
    s->endArray();

    // Create order map
    fallback_order_.clear();
    uint rank = 1;
    for (auto it = o.rbegin(); it != o.rend(); ++it, ++rank)
        fallback_order_.emplace(*it, rank);
}
