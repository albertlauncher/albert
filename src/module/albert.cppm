// Copyright (c) 2025-2025 Manuel Schneider

module;

#include "albert/app.h"
#include "albert/backgroundexecutor.h"
#include "albert/desktopentryparser.h"
#include "albert/download.h"
#include "albert/extension.h"
#include "albert/extensionplugin.h"
#include "albert/fallbackhandler.h"
#include "albert/frontend.h"
#include "albert/generatorqueryhandler.h"
#include "albert/globalqueryhandler.h"
#include "albert/icon.h"
#include "albert/indexitem.h"
#include "albert/indexqueryhandler.h"
#include "albert/inputhistory.h"
#include "albert/item.h"
#include "albert/matchconfig.h"
#include "albert/matcher.h"
#include "albert/messagebox.h"
#include "albert/networkutil.h"
#include "albert/notification.h"
#include "albert/oauth.h"
#include "albert/oauthconfigwidget.h"
#include "albert/plugindependency.h"
#include "albert/plugininstance.h"
#include "albert/pluginloader.h"
#include "albert/pluginmetadata.h"
#include "albert/pluginprovider.h"
#include "albert/querycontext.h"
#include "albert/queryexecution.h"
#include "albert/queryhandler.h"
#include "albert/queryresults.h"
#include "albert/rankitem.h"
#include "albert/ratelimiter.h"
#include "albert/standarditem.h"
#include "albert/systemutil.h"
#include "albert/telemetryprovider.h"
#include "albert/timeit.h"
#include "albert/urlhandler.h"
#include "albert/widgetsutil.h"

// #include "albert/logging.h" // TODO spdlog

// NOTE: THIS IS EXPERIMENTAL API AND MAY CHANGE WITHOUT WARNING IN MINOR RELEASES!

export module albert;

// -------------------------------------------------------------------------------------------------
// Core
// -------------------------------------------------------------------------------------------------

export namespace albert{

// Core types

using albert::Action;
using albert::App;
using albert::Dependency;
using albert::Extension;
using albert::FallbackHandler;
using albert::GeneratorQueryHandler;
using albert::GlobalQueryHandler;
using albert::Icon;
using albert::Item;
using albert::PluginInstance;
using albert::PluginLoader;
using albert::PluginMetadata;
using albert::PluginProvider;
using albert::QueryContext;
using albert::QueryExecution;
using albert::QueryHandler;
using albert::QueryResult;
using albert::QueryResults;
using albert::RankItem;
using albert::StrongDependency;
using albert::UrlHandler;
using albert::WeakDependency;

// -------------------------------------------------------------------------------------------------
// Utility
// -------------------------------------------------------------------------------------------------


// Core utilities
using albert::BackgroundExecutor;
using albert::ExtensionPlugin;
using albert::IndexItem;
using albert::IndexQueryHandler;
using albert::Match;
using albert::MatchConfig;
using albert::Matcher;
using albert::StandardItem;

// Network utilities
using albert::Download;
using albert::OAuth2;
using albert::network;
using albert::await;
using albert::percentEncoded;
using albert::percentDecoded;

// UI utilities
using albert::OAuthConfigWidget;
using albert::question;
using albert::information;
using albert::warning;
using albert::critical;

// System utilities
using albert::Notification;
using albert::openUrl;
using albert::open;
using albert::havePasteSupport;
using albert::setClipboardText;
using albert::setClipboardTextAndPaste;
using albert::runDetachedProcess;
using albert::toQString;
#ifdef Q_OS_MAC
using albert::runAppleScript;
#endif

// Widget binding utilities
using albert::bindWidget;

}

// -------------------------------------------------------------------------------------------------
// Private API
// -------------------------------------------------------------------------------------------------

using albert::detail::Frontend;
using albert::detail::DesktopEntryParser;
using albert::detail::InputHistory;
using albert::detail::DynamicItem;
using albert::detail::TelemetryProvider;
using albert::detail::TimeIt;
using albert::detail::RateLimiter;

// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------

// THESE HAVE TO BE PORTED

// Logging system
// Q_DECLARE_LOGGING_CATEGORY(AlbertLoggingCategory)
// #define ALBERT_LOGGING_CATEGORY(name) Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert." name)
// #define DEBG qCDebug(AlbertLoggingCategory,).noquote()
// #define INFO qCInfo(AlbertLoggingCategory,).noquote()
// #define WARN qCWarning(AlbertLoggingCategory,).noquote()
// #define CRIT qCCritical(AlbertLoggingCategory,).noquote()

// Plugin declaration macro
// #define ALBERT_PLUGIN Q_OBJECT Q_PLUGIN_METADATA(IID ALBERT_PLUGIN_IID FILE "metadata.json")

