// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/action.h"
#include "albert/albert.h"
#include "albert/config.h"
#include "albert/export.h"
#include "albert/extension.h"
#include "albert/extensionregistry.h"
#include "albert/extensionwatcher.h"
#include "albert/fallbackprovider.h"
#include "albert/frontend.h"
#include "albert/history.h"
#include "albert/item.h"
#include "albert/logging.h"
#include "albert/plugin.h"
#include "albert/pluginprovider.h"
#include "albert/query.h"
#include "albert/queryhandler.h"
#include "albert/settingswidgetprovider.h"
#include "albert/standardactions.h"
#include "albert/standarditem.h"


#define ALBERT_PLUGIN_METADATA(file) Q_PLUGIN_METADATA(IID ALBERT_IID FILE file)
#define ALBERT_PLUGIN ALBERT_DEFINE_LOGGING_CATEGORY ALBERT_PLUGIN_METADATA("metadata.json")


//Albert;
//#include <QJsonDocument>
//#include <QJsonObject>
//#include <QCoreApplication>
//#define STR(x) #x
//
//#define ALBERT_PLUGIN_DEFINE_METADATA \
//    protected: const QJsonObject metadata = QJsonDocument::fromJson(QCoreApplication::translate(PROJECT_NAME, STR(PROJECT_METADATA)).toUtf8()).object();
////#define ALBERT_PLUGIN_DEFINE_METADATA_GETTER(func_name, metadata_key) \
////    public: QString func_name() const override { static QString value = metadata.value(#metadata_key).toString(); return value; };
