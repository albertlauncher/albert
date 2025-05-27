// Copyright (c) 2022-2025 Manuel Schneider

#include "albert.h"
#include "app.h"
#include "frontend.h"
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

inline static filesystem::path getFilesystemPath(QStandardPaths::StandardLocation loc)
{ return filesystem::path(QStandardPaths::writableLocation(loc).toStdString()); }

filesystem::path albert::cacheLocation()
{ return getFilesystemPath(QStandardPaths::CacheLocation); }

filesystem::path albert::configLocation()
{ return getFilesystemPath(QStandardPaths::AppConfigLocation); }

filesystem::path albert::dataLocation()
{ return getFilesystemPath(QStandardPaths::AppDataLocation); }

inline static unique_ptr<QSettings> settingsFromPath(const filesystem::path &path)
{ return make_unique<QSettings>(path.string().data(), QSettings::IniFormat); }

unique_ptr<QSettings> albert::settings()
{ return settingsFromPath(configLocation() / "config"); }

unique_ptr<QSettings> albert::state()
{ return settingsFromPath(cacheLocation() / "state"); }

void albert::showSettings(QString plugin_id)
{ App::instance()->showSettings(plugin_id); }

const albert::ExtensionRegistry &albert::extensionRegistry()
{ return App::instance()->extensionRegistry(); }
