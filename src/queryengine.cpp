// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/logging.h"
#include "globalqueryhandlerprivate.h"
#include "indexqueryhandlerprivate.h"
#include "itemsmodel.h"
#include "query.h"
#include "queryengine.h"
#include "triggerqueryhandlerprivate.h"
#include "usagedatabase.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
#include <cmath>
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

shared_ptr<Query> QueryEngine::query(const QString &query_string)
{
    shared_ptr<QueryBase> query;
    vector<FallbackHandler*> fhandlers;
    for (const auto&[id, handler] : enabled_fallback_handlers_)
        fhandlers.emplace_back(handler);

    for (const auto &[trigger, handler] : active_triggers_)
        if (query_string.startsWith(trigger))
            return query = make_shared<TriggerQuery>(::move(fhandlers), handler, query_string.mid(trigger.size()), trigger);

    {
        vector<GlobalQueryHandler*> ghandlers;
        for (const auto&[id, handler] : enabled_global_handlers_)
            ghandlers.emplace_back(handler);
        return query = make_shared<GlobalQuery>(::move(fhandlers), (!query_string.isEmpty() || runEmptyQuery_) ? ::move(ghandlers) : vector<GlobalQueryHandler*>(), query_string);
    }
}

std::map<QString, TriggerQueryHandler*> QueryEngine::triggerHandlers()
{ return registry_.extensions<TriggerQueryHandler>(); }

std::map<QString, GlobalQueryHandler*> QueryEngine::globalHandlers()
{ return registry_.extensions<GlobalQueryHandler>(); }

std::map<QString, FallbackHandler*> QueryEngine::fallbackHandlers()
{ return registry_.extensions<FallbackHandler>(); }

bool QueryEngine::isActive(TriggerQueryHandler *handler) const
{ return enabled_trigger_handlers_.contains(handler->id()); }

bool QueryEngine::isActive(GlobalQueryHandler *handler) const
{ return enabled_global_handlers_.contains(handler->id()); }

bool QueryEngine::isActive(FallbackHandler *handler) const
{ return enabled_fallback_handlers_.contains(handler->id()); }

QString QueryEngine::setActive(TriggerQueryHandler *handler, bool activate)
{
    if (isActive(handler) == activate)
        return {};

    if (activate) {
        // try register trigger
        if (const auto&[t_it, tsuccess] = active_triggers_.emplace(handler->trigger(), handler); tsuccess) {
            // register handler
            if (const auto&[h_it, hsuccess] = enabled_trigger_handlers_.emplace(handler->id(), handler); !hsuccess) {
                active_triggers_.erase(t_it); // cleanup
                return QString("Trigger handler already registered: '%1'.").arg(handler->id());
            }
        } else
            return QString("Trigger '%1' is reserved for '%2'.").arg(handler->trigger(), t_it->second->id());
    } else {
        // remove trigger only if was registered
        if (isActive(handler)){
            active_triggers_.erase(handler->trigger());
            enabled_trigger_handlers_.erase(handler->id());
        }
    }
    return {};
}

void QueryEngine::setActive(GlobalQueryHandler *handler, bool activate)
{
    if (activate)
        enabled_global_handlers_.emplace(handler->id(), handler);
    else
        enabled_global_handlers_.erase(handler->id());
}

void QueryEngine::setActive(FallbackHandler *handler, bool activate)
{
    if (activate)
        enabled_fallback_handlers_.emplace(handler->id(), handler);
    else
        enabled_fallback_handlers_.erase(handler->id());
}

bool QueryEngine::isEnabled(TriggerQueryHandler *handler) const
{ return settings()->value(QString("%1/%2").arg(handler->id(), CFG_THANDLER_ENABLED), true).toBool(); }

bool QueryEngine::isEnabled(GlobalQueryHandler *handler) const
{ return settings()->value(QString("%1/%2").arg(handler->id(), CFG_GHANDLER_ENABLED), true).toBool(); }

bool QueryEngine::isEnabled(FallbackHandler *handler) const
{ return settings()->value(QString("%1/%2").arg(handler->id(), CFG_FHANDLER_ENABLED),  true).toBool(); }

QString QueryEngine::setEnabled(TriggerQueryHandler *handler, bool enable)
{
    settings()->setValue(QString("%1/%2").arg(handler->id(), CFG_THANDLER_ENABLED), enable);
    return setActive(handler, enable);
}

void QueryEngine::setEnabled(GlobalQueryHandler *handler, bool enable)
{
    settings()->setValue(QString("%1/%2").arg(handler->id(), CFG_GHANDLER_ENABLED), enable);
    setActive(handler, enable);
}

void QueryEngine::setEnabled(FallbackHandler *handler, bool enable)
{
    settings()->setValue(QString("%1/%2").arg(handler->id(), CFG_FHANDLER_ENABLED), enable);
    setActive(handler, enable);
}

QString QueryEngine::setTrigger(TriggerQueryHandler *handler, const QString& trigger)
{
    if (handler->trigger() != trigger) {
        if (handler->allowTriggerRemap()) {
            setActive(handler, false);
            if (trigger.isEmpty()){
                handler->d->trigger = handler->defaultTrigger();
                settings()->remove(QString("%1/%2").arg(handler->id(), CFG_TRIGGER));
            } else {
                handler->d->trigger = trigger;
                settings()->setValue(QString("%1/%2").arg(handler->id(), CFG_TRIGGER), trigger);
            }
            return setActive(handler);
        } else
           return QString("'%1' does not allow to remap trigger.").arg(handler->id());
    } else
        return {};
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

bool QueryEngine::runEmptyQuery() const
{ return runEmptyQuery_; }

void QueryEngine::setRunEmptyQuery(bool value)
{ settings()->setValue(CFG_RUN_EMPTY_QUERY, runEmptyQuery_ = value); }

void QueryEngine::onAdd(TriggerQueryHandler *handler)
{
    handler->d->trigger = settings()->value(QString("%1/%2").arg(handler->id(), CFG_TRIGGER), handler->defaultTrigger()).toString();
    handler->setFuzzyMatching(settings()->value(QString("%1/%2").arg(handler->id(), CFG_FUZZY), false).toBool());
    if (isEnabled(handler))
        if(auto err = setActive(handler); !err.isNull())
            WARN << QString("Failed enabling trigger handler '%1': %2").arg(handler->id(), err);
}

void QueryEngine::onAdd(GlobalQueryHandler *handler)
{ if (isEnabled(handler)) setActive(handler); }

void QueryEngine::onAdd(FallbackHandler *handler)
{ if (isEnabled(handler)) setActive(handler); }

void QueryEngine::onRem(TriggerQueryHandler *handler) { setActive(handler, false); }

void QueryEngine::onRem(GlobalQueryHandler *handler) { setActive(handler, false); }

void QueryEngine::onRem(FallbackHandler *handler) { setActive(handler, false); }
