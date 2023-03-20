// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "albert/util/util.h"
#include <QClipboard>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QProcess>
#include <QUrl>

static const char *website_url = "https://albertlauncher.github.io/";
static const char *issue_tracker_url = "https://github.com/albertlauncher/albert/issues";


void albert::openUrl(const QString &url)
{
    DEBG << QString("Opening URL '%1'").arg(url);
    if (!QDesktopServices::openUrl(QUrl(url)))
        WARN << "Failed opening URL" << url;
}

void albert::openWebsite()
{
    QDesktopServices::openUrl(QUrl(website_url));
}

void albert::openIssueTracker()
{
    QDesktopServices::openUrl(QUrl(issue_tracker_url));
}

void albert::setClipboardText(const QString &text)
{
    QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard);
    QGuiApplication::clipboard()->setText(text, QClipboard::Selection);
}

long long albert::runDetachedProcess(const QStringList &commandline, const QString &working_dir)
{
    qint64 pid = 0;
    if (!commandline.empty()) {
        if (QProcess::startDetached(commandline[0], commandline.mid(1), working_dir, &pid))
            INFO << "Detached process started successfully. PID:" << pid << commandline;
        else
            WARN << "Starting detached process failed." << commandline;
    } else
        WARN << "runDetachedProcess: commandline must not be empty!";
    return pid;
}
