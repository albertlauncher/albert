// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "app.h"

albert::ExtensionRegistry &albert::extensionRegistry()
{
    return App::instance()->extension_registry;
}

albert::Query *albert::query(const QString &query)
{
    return App::instance()->query_engine->query(query);
}

void albert::show(const QString &text)
{
    auto frontend = App::instance()->frontend;
    if (!text.isNull())
        frontend->setInput(text);
    frontend->setVisible(true);
}

void albert::showSettings()
{
    App::instance()->showSettings();
}

void albert::restart()
{
    QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1));
}

void albert::quit()
{
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}
