// Copyright (c) 2022-2025 Manuel Schneider

#include "albert.h"
#include "app.h"
#include "frontend.h"
#include "queryengine.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
using namespace std;


void albert::show(const QString &input_text)
{
    if (!input_text.isNull())
        App::instance()->frontend()->setInput(input_text);
    App::instance()->frontend()->setVisible(true);
}

void albert::restart()
{ QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1)); }

void albert::quit()
{ QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection); }

static filesystem::path getFilesystemPath(QStandardPaths::StandardLocation loc)
{ return filesystem::path(QStandardPaths::writableLocation(loc).toStdString()); }

const filesystem::path &albert::cacheLocation()
{
    static const auto p = getFilesystemPath(QStandardPaths::CacheLocation);
    return p;
}

const filesystem::path &albert::configLocation()
{
    static const auto p = getFilesystemPath(QStandardPaths::AppConfigLocation);
    return p;
}

const filesystem::path &albert::dataLocation()
{
    static const auto p = getFilesystemPath(QStandardPaths::AppDataLocation);
    return p;
}

unique_ptr<QSettings> albert::settings()
{
    const auto path = QString::fromStdString((configLocation() / "config").string());
    return make_unique<QSettings>(path, QSettings::IniFormat);
}

unique_ptr<QSettings> albert::state()
{
    const auto path = QString::fromStdString((dataLocation() / "state").string());
    return make_unique<QSettings>(path, QSettings::IniFormat);
}

void albert::showSettings(QString plugin_id)
{ App::instance()->showSettings(plugin_id); }

const albert::ExtensionRegistry &albert::extensionRegistry()
{ return App::instance()->extensionRegistry(); }
