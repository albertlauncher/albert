// Copyright (C) 2014-2019 Manuel Schneider

#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QProcess>
#include <QStringList>
#include <pwd.h>
#include <unistd.h>
#include "standardactions.h"

namespace  {

void runDetached(const QStringList &commandline, const QString &workingDirectory) {
    if (commandline.isEmpty()){
        qDebug() << "NOP: commandline is empty";
    } else {
        qint64 pid;
        QStringList mutableCmdLn(commandline);
        if (QProcess::startDetached(mutableCmdLn.takeFirst(), mutableCmdLn, workingDirectory, &pid))
            qInfo() << "Detached process started successfully. PID:" << pid << commandline;
        else
            qWarning() << "Starting detached process failed." << commandline;
    }
}

}

// Place it somewhere for now. TODO: move it to potential core plugin
EXPORT_CORE QString terminalCommand;

/** **************************************************************************/
Core::StandardActionBase::StandardActionBase(const QString &text)
    : text_(std::move(text)) { }

QString Core::StandardActionBase::text() const {
    return text_;
}


/** **************************************************************************/
Core::FuncAction::FuncAction(const QString &text, std::function<void ()> action)
    : StandardActionBase(text), action_(std::move(action)) { }

void Core::FuncAction::activate() {
    action_();
}


/** **************************************************************************/
Core::ClipAction::ClipAction(const QString &text, QString clipBoardText)
    : StandardActionBase(text), clipBoardText_(std::move(clipBoardText)) { }

void Core::ClipAction::activate() {
    QGuiApplication::clipboard()->setText(clipBoardText_);
}


/** **************************************************************************/
Core::UrlAction::UrlAction(const QString &text, QUrl url)
    : StandardActionBase(text), url_(std::move(url)) { }

void Core::UrlAction::activate() {
    QDesktopServices::openUrl(url_);
}


/** **************************************************************************/
Core::ProcAction::ProcAction(const QString &text, const QStringList &commandline, const QString &workingDirectory)
    : StandardActionBase(text), commandline_(commandline), workingDir_(workingDirectory) { }

void Core::ProcAction::activate() {
    runDetached(commandline_, workingDir_);
}

/** **************************************************************************/
Core::TermAction::TermAction(const QString &text, const QStringList &commandline, const QString &workingDirectory)
    : StandardActionBase(text), commandline_(commandline), workingDir_(workingDirectory) { }

void Core::TermAction::activate() {
    QStringList commandline = terminalCommand.split(QChar(QChar::Space), QString::SkipEmptyParts);
    commandline.append(commandline_);
    runDetached(commandline, workingDir_);
}

/** **************************************************************************/
Core::ShTermAction::ShTermAction(const QString &text, const QString &script, CloseBehavior closeBehavior, const QString &workingDirectory)
    : StandardActionBase(text), script_(script), closeBehavior_(closeBehavior), workingDir_(workingDirectory) { }

void Core::ShTermAction::activate() {

    // Get the user shell (passwd must not be freed)
    passwd *pwd = getpwuid(geteuid());
    if (pwd == nullptr)
        throw "Could not retrieve user shell";

    QStringList commandline;
    switch (closeBehavior_) {
    case CloseBehavior::CloseOnExit:
        commandline << pwd->pw_shell << "-ic" << script_;
        break;
    case CloseBehavior::CloseOnSuccess:
        commandline << pwd->pw_shell << "-ic" << QString("%1 || exec %1").arg(script_, pwd->pw_shell);
        break;
    case CloseBehavior::DoNotClose:
        commandline << pwd->pw_shell << "-ic" << QString("%1; exec %1").arg(script_, pwd->pw_shell);
        break;
    }

    runDetached(commandline, workingDir_);
}
