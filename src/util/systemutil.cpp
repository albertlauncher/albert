// Copyright (c) 2025-2025 Manuel Schneider

#include "logging.h"
#include "systemutil.h"
#include "messagebox.h"
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>
using namespace albert::util;
using namespace albert;
using namespace std;

void util::openUrl(const QString &url)
{
    if (QUrl qurl(url); qurl.isValid())
        open(QUrl(url));
    else
        WARN << "Invalid URL" << url << qurl.errorString();
}

void util::open(const QUrl &url)
{
    DEBG << QString("Open URL '%1'").arg(url.toString());

    if (qApp->platformName() == "wayland")
        runDetachedProcess({"xdg-open", url.toString()});
    else if (!QDesktopServices::openUrl(url))
        WARN << "Failed to open URL" << url;
}

void util::open(const QString &path) { open(QUrl::fromLocalFile(path)); }

void util::open(const filesystem::path &path) { open(QString::fromLocal8Bit(path.native())); }


void util::setClipboardText(const QString &text)
{
    QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard);
    QGuiApplication::clipboard()->setText(text, QClipboard::Selection);
}

static bool checkPasteSupport()
{
#if defined Q_OS_MACOS
    return !QStandardPaths::findExecutable("osascript").isEmpty();
#elif defined(Q_OS_UNIX)
    bool have_paste_support = !QStandardPaths::findExecutable("xdotool").isEmpty();
    if(!have_paste_support)
        WARN << "xdotool is not available. No paste support.";
    else if(qgetenv("XDG_SESSION_TYPE") != "x11")
        WARN << "xdotool is available but but session type is not x11. "
                "Paste will work for X11 windows only.";
    return have_paste_support;
#endif
}

bool util::havePasteSupport()
{
    static bool have_paste_support = checkPasteSupport();
    return have_paste_support;
}

void util::setClipboardTextAndPaste(const QString &text)
{
    setClipboardText(text);
    if (!havePasteSupport())
    {
        auto t = "Received a request to paste, although the feature is not supported. "
                 "Looks like the plugin did not check for feature support before. "
                 "Please report this issue.";
        WARN << t;
        warning(t);
        return;
    }

#if defined(Q_OS_MACOS)
    runDetachedProcess({
        "osascript", "-e",
        R"(tell application "System Events" to keystroke "v" using command down)"
    });
#elif defined(Q_OS_UNIX)
    QCoreApplication::processEvents(); // ??
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
#elif defined(Q_OS_WIN)
    qFatal("Paste not implemented on windows.");
#endif
}

long long util::runDetachedProcess(const QStringList &commandline, const QString &working_dir)
{
    qint64 pid = 0;
    if (!commandline.empty())
    {
        auto wd = working_dir.isEmpty() ? QDir::homePath() : working_dir;
        if (QProcess::startDetached(commandline[0], commandline.mid(1), wd, &pid))
            INFO << QString("Detached process started successfully. (WD: %1, PID: %2, CMD: %3")
                        .arg(wd).arg(pid).arg(QDebug::toString(commandline));
        else
            WARN << "Starting detached process failed." << commandline;
    } else
        WARN << "runDetachedProcess: commandline must not be empty!";
    return pid;
}

long long util::runDetachedProcess(const QStringList &commandline)
{ return runDetachedProcess(commandline, {}); }

void util::tryCreateDirectory(const filesystem::path& path)
{
    try {
        filesystem::create_directories(path);
    } catch (const filesystem::filesystem_error &e) {
        throw runtime_error(
            QCoreApplication::translate("albert", "Failed creating directory %1: %2")
                .arg(e.path1().c_str(), e.what()).toStdString());
    }
}
