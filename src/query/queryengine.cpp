// Copyright (c) 2022-2025 Manuel Schneider

#include "albert/app.h"
#include "extensionregistry.h"
#include "fallbackhandler.h"
#include "globalqueryhandler.h"
#include "logging.h"
#include "queryengine.h"
#include "queryresults.h"
#include "queryprivate.h"
#include "usagedatabase.h"
#include "usagescoring.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
using namespace Qt::StringLiterals;
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


QueryEngine::QueryEngine(ExtensionRegistry &registry)
    : registry_(registry)
    , usage_scoring_(0,0,{})  // Null scoring, just to not have to implement constructors
{
    auto s = App::settings();

    auto decay = s->value(CFG_MEMORY_DECAY, DEF_MEMORY_DECAY).toDouble();
    auto prioritize_perfect_match = s->value(CFG_PRIO_PERFECT, DEF_PRIO_PERFECT).toBool();
    usage_scoring_ = UsageScoring(prioritize_perfect_match, decay,
                                  make_shared<unordered_map<ItemKey, double>>
                                  (UsageDatabase::instance().itemUsageScores(decay)));

    loadFallbackOrder();

    connect(&registry, &ExtensionRegistry::added, this, [this](Extension *e)
    {
        const auto id = e->id();
        auto settings = App::instance().settings();
        settings->beginGroup(id);

        if (auto *h = dynamic_cast<albert::QueryHandler*>(e))
        {
            auto t = settings->value(CFG_TRIGGER, h->defaultTrigger()).toString();
            auto f = settings->value(CFG_FUZZY, false).toBool();

            h->setTrigger(t);
            h->setFuzzyMatching(f);
            trigger_handlers_.emplace(piecewise_construct,
                                      forward_as_tuple(id),
                                      forward_as_tuple(h, t, f));
            emit queryHandlerAdded(h);

            updateActiveTriggers();
        }

        if (auto *h = dynamic_cast<albert::GlobalQueryHandler*>(e))
        {
            global_handlers_.emplace(id, h);
            if (settings->value(CFG_GLOBAL_HANDLER_ENABLED, true).toBool())
                global_query_.global_query_handlers.emplace(id, h);

            emit globalQueryHandlerAdded(h);
        }

        if (auto *h = dynamic_cast<albert::FallbackHandler*>(e))
        {
            fallback_handlers_.emplace(id, h);
            emit fallbackHandlerAdded(h);
        }
    });

    connect(&registry, &ExtensionRegistry::removed, this, [this](Extension *e)
    {
        const auto id = e->id();

        if (const auto it = trigger_handlers_.find(id); it != trigger_handlers_.end())
        {
            auto h = it->second.handler;
            trigger_handlers_.erase(it);
            emit queryHandlerRemoved(h);
            updateActiveTriggers();
        }

        if (const auto it = global_handlers_.find(id); it != global_handlers_.end())
        {
            auto h = it->second;
            global_handlers_.erase(it);
            global_query_.global_query_handlers.erase(id);
            emit globalQueryHandlerRemoved(h);
        }

        if (const auto it = fallback_handlers_.find(id); it != fallback_handlers_.end())
        {
            auto h = it->second;
            fallback_handlers_.erase(it);
            emit fallbackHandlerRemoved(h);
        }
    });
}

void QueryEngine::setMemoryDecay(double v)
{
    if (usage_scoring_.memory_decay != v)
    {
        DEBG << "memoryDecay set to" << v;
        App::settings()->setValue(CFG_MEMORY_DECAY, v);
        usage_scoring_ = UsageScoring(usage_scoring_.prioritize_perfect_match, v,
                                      make_shared<unordered_map<ItemKey, double>>
                                      (UsageDatabase::instance().itemUsageScores(v)));
    }
}

void QueryEngine::setPrioritizePerfectMatch(bool v)
{
    if (usage_scoring_.prioritize_perfect_match != v)
    {
        DEBG << "prioritizePerfectMatch set to" << v;
        App::settings()->setValue(CFG_PRIO_PERFECT, v);
        usage_scoring_ = UsageScoring(v, usage_scoring_.memory_decay, usage_scoring_.usage_scores);
    }
}

void QueryEngine::storeItemActivation(const QString &query, const QString &extension,
                                      const QString &item, const QString &action)
{
    UsageDatabase::instance().addActivation(query, extension, item, action);

    auto scores = UsageDatabase::instance().itemUsageScores(usage_scoring_.memory_decay);

    usage_scoring_ = UsageScoring(
        usage_scoring_.prioritize_perfect_match,
        usage_scoring_.memory_decay,
        make_shared<unordered_map<ItemKey, double>>(::move(scores))
    );
}

UsageScoring QueryEngine::usageScoring() const
{
    return usage_scoring_;
}

unique_ptr<detail::Query> QueryEngine::query(QString string)
{
    vector<QueryResult> fallbacks;
    if (!string.isEmpty())
        fallbacks = this->fallbacks(string);

    QString trigger;
    albert::QueryHandler *handler;
    if (auto it = ranges::find_if(active_triggers_.cbegin(), active_triggers_.cend(),
                                  [&](const auto &t){ return string.startsWith(t.first); });
        it != active_triggers_.cend())
    {
        trigger = it->first;
        handler = it->second;
        string = string.mid(trigger.size());
    }
    else
        handler = &global_query_;

    auto query = unique_ptr<detail::Query>(
        new detail::Query(usage_scoring_, ::move(fallbacks), *handler, trigger, string));

    connect(&query->matches(), &QueryResults::resultActivated,
            this, &QueryEngine::storeItemActivation);

    connect(&query->fallbacks(), &QueryResults::resultActivated,
            this, &QueryEngine::storeItemActivation);

    return query;
}

//
// Trigger handlers
//

map<QString, QueryHandler*> QueryEngine::triggerHandlers()
{
    map<QString, albert::QueryHandler*> handlers;
    for (const auto &[id, h] : trigger_handlers_)
        handlers.emplace(id, h.handler);
    return handlers;
}

const map<QString, QueryHandler *> &QueryEngine::activeTriggerHandlers() const
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
        App::settings()->remove(QString("%1/%2").arg(id, CFG_TRIGGER));
    }
    else
    {
        h.trigger = t;
        App::settings()->setValue(QString("%1/%2").arg(id, CFG_TRIGGER), t);
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
        App::settings()->setValue(QString("%1/%2").arg(id, CFG_FUZZY), f);
        h.handler->setFuzzyMatching(f);
    }
}


//
// Global handlers
//

map<QString, GlobalQueryHandler*> QueryEngine::globalHandlers()
{
    return global_handlers_;
    // map<QString, albert::GlobalQueryHandler*> handlers;
    // for (const auto &[id, h] : global_handlers_)
    //     handlers.emplace(id, h.handler);
    // return handlers;
}

bool QueryEngine::isEnabled(const QString &id) const
{ return global_query_.global_query_handlers.contains(id); }

void QueryEngine::setEnabled(const QString &id, bool e)
{
    auto *h = global_handlers_.at(id);

    if (isEnabled(id) != e)
    {
        App::settings()->setValue(QString("%1/%2").arg(id, CFG_GLOBAL_HANDLER_ENABLED), e);
        if (e)
            global_query_.global_query_handlers.emplace(id, h);
        else
            global_query_.global_query_handlers.erase(id);
    }
}


//
// Fallback handlers
//

map<QString, FallbackHandler*> QueryEngine::fallbackHandlers()
{ return fallback_handlers_; }

const map<pair<QString,QString>,int> &QueryEngine::fallbackOrder() const
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
    auto s = App::settings();
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
    auto s = App::settings();
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

vector<QueryResult> QueryEngine::fallbacks(const QString &query)
{
    vector<pair<FallbackHandler*, RankItem>> fallbacks;

    for (auto &[id, fallback_handler] : fallback_handlers_)
        for (auto item : fallback_handler->fallbacks(query))
            if (auto it = fallbackOrder().find(make_pair(id, item->id()));
                it == fallbackOrder().end())
                fallbacks.emplace_back(fallback_handler, RankItem(::move(item), 0));
            else
                fallbacks.emplace_back(fallback_handler, RankItem(::move(item), it->second));

    ranges::sort(fallbacks, greater(), &decltype(fallbacks)::value_type::second);

    auto view = fallbacks | views::transform([](auto &p) {
                         return QueryResult{p.first, ::move(p.second.item)};
                     });

    return {begin(view), end(view)};
}
