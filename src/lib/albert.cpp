// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "app.h"
#include "logging.h"
#include <QDesktopServices>
#include <QGuiApplication>
#include <QProcess>
#include <QClipboard>
#include <QUrl>

static const char *website_url = "https://albertlauncher.github.io/";
static const char *issue_tracker_url = "https://github.com/albertlauncher/albert/issues";

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

void albert::openWebsite()
{
    QDesktopServices::openUrl(QUrl(website_url));
}

void albert::openIssueTracker()
{
    QDesktopServices::openUrl(QUrl(issue_tracker_url));
}

int albert::runDetachedProcess(const QStringList &commandline, const QString &working_dir)
{
    qint64 pid = 0;
    if (commandline.size() > 0) {
        if (QProcess::startDetached(commandline[0], commandline.mid(1), working_dir, &pid))
            INFO << "Detached process started successfully. PID:" << pid << commandline;
        else
            WARN << "Starting detached process failed." << commandline;
    }
    WARN << "runDetachedProcess: commandline must not be empty!";
    return pid;
}

void albert::setClipboardText(const QString &text)
{
    QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard);
    QGuiApplication::clipboard()->setText(text, QClipboard::Selection);
}