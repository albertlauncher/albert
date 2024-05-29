// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/util.h"
#include "albert/logging.h"
#include "app.h"
#include "terminalprovider.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QProcess>
#include <memory>
#include <QSettings>
#include <QStandardPaths>
using namespace std;


QNetworkAccessManager *albert::network()
{
    static QNetworkAccessManager network_manager;
    return &network_manager;
}

QString albert::configLocation()
{ return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) ;}

QString albert::dataLocation()
{ return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) ;}

QString albert::cacheLocation()
{ return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) ;}

std::unique_ptr<QSettings> albert::settings()
{ return make_unique<QSettings>(QString("%1/%2").arg(configLocation(), "config"), QSettings::IniFormat); }

std::unique_ptr<QSettings> albert::state()
{ return make_unique<QSettings>(QString("%1/%2").arg(cacheLocation(), "state"), QSettings::IniFormat); }

void albert::runTerminal(const QString &script, const QString &working_dir, bool close_on_exit)
{ App::instance()->terminal().run(script, working_dir, close_on_exit); }

void albert::showSettings(QString plugin_id)
{ App::instance()->showSettings(plugin_id); }

void albert::openUrl(const QString &url)
{ openUrl(QUrl(url)); }

void albert::openUrl(const QUrl &url)
{
    DEBG << QString("Opening URL '%1'").arg(url.toString());
    if (qApp->platformName() == "wayland")
        runDetachedProcess({"xdg-open", url.toString()});
    else if (!QDesktopServices::openUrl(url))
        WARN << "Failed opening URL" << url;
}

void albert::openWebsite()
{
    static const char *website_url = "https://albertlauncher.github.io/";
    QDesktopServices::openUrl(QUrl(website_url));
}

void albert::setClipboardText(const QString &text)
{
    QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard);
    QGuiApplication::clipboard()->setText(text, QClipboard::Selection);
}

static bool checkPasteSupport()
{
#if defined Q_OS_MACOS
    return !QStandardPaths::findExecutable("osascript").isEmpty();
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    bool have_paste_support = !QStandardPaths::findExecutable("xdotool").isEmpty();
    if(!have_paste_support)
        WARN << "xdotool is not available. No paste support.";
    else if(qgetenv("XDG_SESSION_TYPE") != "x11")
        WARN << "xdotool is available but but session type is not x11. "
                "Paste will work for X11 windows only.";
    return have_paste_support;
#endif
}

bool albert::havePasteSupport()
{
    static bool have_paste_support = checkPasteSupport();
    return have_paste_support;
}

void albert::setClipboardTextAndPaste(const QString &text)
{
    setClipboardText(text);
    if (!havePasteSupport())
    {
        auto t = "Received a request to paste, although the feature is not supported. "
                 "Looks like the plugin did not check for feature support before. "
                 "Please report this issue.";
        WARN << t;
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(), t);
        return;
    }

#if defined Q_OS_MACOS
    runDetachedProcess({
        "osascript", "-e",
        R"(tell application "System Events" to keystroke "v" using command down)"
    });
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    QApplication::processEvents(); // ??
    auto *proc = new QProcess;
    proc->start("sh" , {"-c", "sleep 0.1 && xdotool key ctrl+v"});
    QObject::connect(proc, &QProcess::finished, proc, [proc](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitStatus != QProcess::ExitStatus::NormalExit || exitCode != EXIT_SUCCESS)
        {
            WARN << QString("Paste failed (%1).").arg(exitCode);
            if (auto out = proc->readAllStandardOutput(); out.isEmpty())
                WARN << out;
            if (auto err = proc->readAllStandardError(); err.isEmpty())
                WARN << err;
        }
        proc->deleteLater();
    });
#elif defined Q_OS_WIN
    qFatal("Paste not implemented on windows.");
#endif
}

long long albert::runDetachedProcess(const QStringList &commandline, const QString &working_dir)
{
    qint64 pid = 0;
    if (!commandline.empty()) {
        if (QProcess::startDetached(commandline[0], commandline.mid(1), working_dir.isNull() ? QDir::homePath() : working_dir, &pid))
            INFO << "Detached process started successfully. PID:" << pid << commandline;
        else
            WARN << "Starting detached process failed." << commandline;
    } else
        WARN << "runDetachedProcess: commandline must not be empty!";
    return pid;
}
