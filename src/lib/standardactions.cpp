// Copyright (C) 2014-2019 Manuel Schneider

#include <QGuiApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QProcess>
#include <QStringList>
#include <pwd.h>
#include <unistd.h>
#include "albert/util/shutil.h"
#include "standardactions.h"

// Place it somewhere for now. TODO: move it to potential core plugin
EXPORT_CORE QString terminalCommand;

/** **************************************************************************/
Core::StandardActionBase::StandardActionBase(const QString &text)
    : text_(std::move(text))
{

}

QString Core::StandardActionBase::text() const
{
    return text_;
}


/** **************************************************************************/
Core::FuncAction::FuncAction(const QString &text, std::function<void ()> action)
    : StandardActionBase(text), action_(std::move(action))
{

}

void Core::FuncAction::activate()
{
    action_();
}


/** **************************************************************************/
Core::ClipAction::ClipAction(const QString &text, QString clipBoardText)
    : StandardActionBase(text), clipBoardText_(std::move(clipBoardText))
{

}

void Core::ClipAction::activate()
{
    QGuiApplication::clipboard()->setText(clipBoardText_);
}


/** **************************************************************************/
Core::UrlAction::UrlAction(const QString &text, QUrl url)
    : StandardActionBase(text), url_(std::move(url))
{

}

void Core::UrlAction::activate()
{
    QDesktopServices::openUrl(url_);
}


/** **************************************************************************/
Core::ProcAction::ProcAction(const QString &text, const QStringList &commandline, const QString &workingDirectory)
    : StandardActionBase(text), commandline_(commandline), workingDir_(workingDirectory)
{

}

void Core::ProcAction::activate()
{
    if (commandline_.isEmpty())
        return;

    QStringList commandline = commandline_;
    if (workingDir_.isEmpty())
        QProcess::startDetached(commandline.takeFirst(), commandline);
    else
        QProcess::startDetached(commandline.takeFirst(), commandline, workingDir_);
}


/** **************************************************************************/
Core::TermAction::TermAction(const QString &text, const QStringList &commandline, const QString &workingDirectory, bool shell, Core::TermAction::CloseBehavior behavior)
    : ProcAction (text, commandline, workingDirectory)
{
    if (commandline_.isEmpty())
        return;

    if (shell){

        // Quote the input commandline since it's nested
        QStringList quotedCommandLine;
        for (const QString &strRef : commandline_)
            quotedCommandLine << ShUtil::quote(strRef);

        // Get the user shell (passwd must not be freed)
        passwd *pwd = getpwuid(geteuid());
        if (pwd == nullptr)
            throw "Could not retrieve user shell";

        // Let standard shell handle flow control (syntax differs in shells, e.g. fish)
        if (behavior == CloseBehavior::DoNotClose)
            command_ << "sh" << "-ic" << QString("%1 -ic '%2'; exec %1").arg(pwd->pw_shell, quotedCommandLine.join(' '));
        else if (behavior == CloseBehavior::CloseOnSuccess)
            command_ << "sh" << "-ic" << QString("%1 -ic '%2' && sleep 1 || exec %1").arg(pwd->pw_shell, quotedCommandLine.join(' '));
        else  // behavior_ == CloseBehavior::CloseOnExit
            command_ << "sh" << "-ic" << QString("%1 -ic '%2'; sleep 1 ").arg(pwd->pw_shell, quotedCommandLine.join(' '));
    } else {
        command_ = commandline_;
    }

}

void Core::TermAction::activate()
{
    if (commandline_.isEmpty())
        return;

    commandline_ = Core::ShUtil::split(terminalCommand) + command_;
    ProcAction::activate();
}
