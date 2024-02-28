// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "query.h"
#include "queryengine.h"
#include "triggerqueryhandlerprivate.h"
#include "usagedatabase.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
using namespace albert;
using namespace std;

static const char *CFG_THANDLER_ENABLED = "trigger_handler_enabled";
static const char *CFG_GHANDLER_ENABLED = "global_handler_enabled";
static const char *CFG_FHANDLER_ENABLED = "fallback_hanlder_enabled";
static const char *CFG_TRIGGER = "trigger";
static const char *CFG_FUZZY = "fuzzy";
static const char *CFG_RUN_EMPTY_QUERY = "runEmptyQuery";
static const bool  CFG_RUN_EMPTY_QUERY_DEF = false;

QueryEngine::QueryEngine(ExtensionRegistry &registry):
    ExtensionWatcher<TriggerQueryHandler>(&registry),
    ExtensionWatcher<GlobalQueryHandler>(&registry),
    ExtensionWatcher<FallbackHandler>(&registry),
    registry_(registry)
{
    runEmptyQuery_ = settings()->value(CFG_RUN_EMPTY_QUERY, CFG_RUN_EMPTY_QUERY_DEF).toBool();
    UsageHistory::initialize();
}

unique_ptr<QueryBase> QueryEngine::query(const QString &query_string)
{
    vector<FallbackHandler*> fhandlers;
    for (const auto&[id, handler] : enabled_fallback_handlers_)
        fhandlers.emplace_back(handler);

    for (const auto &[trigger, handler] : active_triggers_)
        if (query_string.startsWith(trigger))
            return make_unique<TriggerQuery>(::move(fhandlers), handler, query_string.mid(trigger.size()), trigger);

    {
        vector<GlobalQueryHandler*> ghandlers;
        for (const auto&[id, handler] : enabled_global_handlers_)
            ghandlers.emplace_back(handler);
        return make_unique<GlobalQuery>(::move(fhandlers), (!query_string.isEmpty() || runEmptyQuery_) ? ::move(ghandlers) : vector<GlobalQueryHandler*>(), query_string);
    }
}


bool QueryEngine::runEmptyQuery() const
{ return runEmptyQuery_; }

void QueryEngine::setRunEmptyQuery(bool value)
{ settings()->setValue(CFG_RUN_EMPTY_QUERY, runEmptyQuery_ = value); }


//
// Trigger handlers
//

map<QString, TriggerQueryHandler*> QueryEngine::triggerHandlers()
{ return registry_.extensions<TriggerQueryHandler>(); }

const map<QString, TriggerQueryHandler *> &QueryEngine::activeTriggerHandlers()
{ return active_triggers_; }

void QueryEngine::updateActiveTriggers()
{
    active_triggers_.clear();
    for (const auto&[hid, h] : enabled_trigger_handlers_)
        if (const auto&[it, success] = active_triggers_.emplace(h->trigger(), h); !success)
            WARN << QString("Trigger '%1' of '%2' already registered for '%3'.")
                        .arg(h->trigger(), hid, it->second->id());
}

void QueryEngine::onAdd(TriggerQueryHandler *h)
{
    h->d->user_trigger = settings()->value(QString("%1/%2").arg(h->id(), CFG_TRIGGER), h->defaultTrigger()).toString();
    h->setFuzzyMatching(settings()->value(QString("%1/%2").arg(h->id(), CFG_FUZZY), false).toBool());

    if (settings()->value(QString("%1/%2").arg(h->id(), CFG_THANDLER_ENABLED), true).toBool())
    {
        enabled_trigger_handlers_.emplace(h->id(), h);
        updateActiveTriggers();
        emit handlersChanged();
    }
}

void QueryEngine::onRem(TriggerQueryHandler *h)
{
    if (enabled_trigger_handlers_.erase(h->id()))
    {
        updateActiveTriggers();
        emit handlersChanged();
    }
}

bool QueryEngine::isEnabled(TriggerQueryHandler *h) const
{ return enabled_trigger_handlers_.contains(h->id()); }

void QueryEngine::setEnabled(TriggerQueryHandler *h, bool e)
{
    if (isEnabled(h) == e)
        return;

    settings()->setValue(QString("%1/%2").arg(h->id(), CFG_THANDLER_ENABLED), e);
    if (e)
        enabled_trigger_handlers_.emplace(h->id(), h);
    else
        enabled_trigger_handlers_.erase(h->id());

    updateActiveTriggers();
    emit handlersChanged();
}

void QueryEngine::setTrigger(TriggerQueryHandler *h, const QString& t)
{
    if (h->trigger() == t || !h->allowTriggerRemap())
        return;

    if (t.isEmpty() || t == h->defaultTrigger())
    {
        h->d->user_trigger = h->defaultTrigger();
        settings()->remove(QString("%1/%2").arg(h->id(), CFG_TRIGGER));
    }
    else
    {
        h->d->user_trigger = t;
        settings()->setValue(QString("%1/%2").arg(h->id(), CFG_TRIGGER), t);
    }

    updateActiveTriggers();
}

bool QueryEngine::fuzzy(TriggerQueryHandler *handler) const
{ return handler->fuzzyMatching(); }

void QueryEngine::setFuzzy(TriggerQueryHandler *handler, bool enable)
{
    if (handler->supportsFuzzyMatching()){
        settings()->setValue(QString("%1/%2").arg(handler->id(), CFG_FUZZY), enable);
        handler->setFuzzyMatching(enable);
    }
}


//
// Global handlers
//

map<QString, GlobalQueryHandler*> QueryEngine::globalHandlers()
{ return registry_.extensions<GlobalQueryHandler>(); }

bool QueryEngine::isEnabled(GlobalQueryHandler *h) const
{ return enabled_global_handlers_.contains(h->id()); }

void QueryEngine::setEnabled(GlobalQueryHandler *h, bool e)
{
    if (isEnabled(h) == e)
        return;

    settings()->setValue(QString("%1/%2").arg(h->id(), CFG_GHANDLER_ENABLED), e);

    if (e)
        enabled_global_handlers_.emplace(h->id(), h);
    else
        enabled_global_handlers_.erase(h->id());

    emit handlersChanged();
}

void QueryEngine::onAdd(GlobalQueryHandler *h)
{
    if (settings()->value(QString("%1/%2").arg(h->id(), CFG_GHANDLER_ENABLED), true).toBool())
    {
        enabled_global_handlers_.emplace(h->id(), h);
        emit handlersChanged();
    }
}

void QueryEngine::onRem(GlobalQueryHandler *h)
{
    if (enabled_global_handlers_.erase(h->id()))
        emit handlersChanged();
}


//
// Fallback handlers
//

map<QString, FallbackHandler*> QueryEngine::fallbackHandlers()
{ return registry_.extensions<FallbackHandler>(); }

map<pair<QString,QString>,int> QueryEngine::fallbackOrder() const
{
    return {};
}

void QueryEngine::setFallbackOrder(map<pair<QString,QString>,int>)
{

}

bool QueryEngine::isEnabled(FallbackHandler *h) const
{ return enabled_fallback_handlers_.contains(h->id()); }

void QueryEngine::setEnabled(FallbackHandler *h, bool e)
{
    if (isEnabled(h) == e)
        return;

    settings()->setValue(QString("%1/%2").arg(h->id(), CFG_FHANDLER_ENABLED), e);

    if (e)
        enabled_fallback_handlers_.emplace(h->id(), h);
    else
        enabled_fallback_handlers_.erase(h->id());

    emit handlersChanged();
}

void QueryEngine::onAdd(FallbackHandler *h)
{
    if (settings()->value(QString("%1/%2").arg(h->id(), CFG_FHANDLER_ENABLED), true).toBool())
    {
        enabled_fallback_handlers_.emplace(h->id(), h);
        emit handlersChanged();
    }
}

void QueryEngine::onRem(FallbackHandler *h)
{
    if (enabled_fallback_handlers_.erase(h->id()))
        emit handlersChanged();
}
