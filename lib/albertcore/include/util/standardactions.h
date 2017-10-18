// Copyright (C) 2014-2017 Manuel Schneider

#pragma once
#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QDesktopServices>
#include <QStringList>
#include <QProcess>
#include <functional>
#include <pwd.h>
#include <unistd.h>
#include "core/action.h"
#include "util/shutil.h"
#include "core_globals.h"

extern QString terminalCommand;

namespace Core {


//! @brief A standard action holding a std::function
struct EXPORT_CORE FuncAction : public Action
{
public:
    FuncAction(QString text, std::function<void()> action)
        : text_(std::move(text)), action_(std::move(action)) { }
    QString text() const override { return text_; }
    void activate() override { action_(); }
private:
    QString text_;
    std::function<void()> action_;
};



// A standard action that copies text into the clipboard
struct EXPORT_CORE ClipAction : public Action
{
public:
    ClipAction(QString text, QString clipBoardText)
        : text_(std::move(text)), clipBoardText_(std::move(clipBoardText)) { }
    QString text() const override { return text_; }
    void activate() override { QApplication::clipboard()->setText(clipBoardText_); }
private:
    QString text_;
    QString clipBoardText_;
};



// A standard action that opens an url using QDesktopServices
struct EXPORT_CORE UrlAction : public Action
{
public:
    UrlAction(QString text, QUrl url)
        : text_(std::move(text)), url_(std::move(url)) { }
    QString text() const override { return text_; }
    void activate() override { QDesktopServices::openUrl(url_); }
private:
    QString text_;
    QUrl url_ ;
};



// A standard action that starts a process
struct EXPORT_CORE ProcAction : public Action
{
public:
    ProcAction(QString text, QStringList commandline, QString workingDirectory = QString())
        : text_(std::move(text)),
          commandline_(std::move(commandline)),
          workingDir_(std::move(workingDirectory)) { }
    QString text() const override { return text_; }
    void activate() override {
        if (commandline_.isEmpty())
            return;
        QStringList commandline = commandline_;
        if (workingDir_.isEmpty())
            QProcess::startDetached(commandline.takeFirst(), commandline);
        else
            QProcess::startDetached(commandline.takeFirst(), commandline, workingDir_);
    }
private:
    QString text_;
    QStringList commandline_;
    QString workingDir_;
};



// A standard action that runs commands in a terminal
struct EXPORT_CORE TermAction : public Action
{
public:
    TermAction(QString text, QStringList commandline, QString workingDirectory = QString(), bool shell = true)
        : text_(std::move(text)),
          commandline_(std::move(commandline)),
          workingDir_(std::move(workingDirectory)),
          shell_(shell) { }
    QString text() const override { return text_; }
    void activate() override {
        QStringList commandline = Core::ShUtil::split(terminalCommand);
        if (shell_){
            // passwd must not be freed
            passwd *pwd = getpwuid(geteuid());
            if (pwd == nullptr)
                throw "Could not retrieve user shell";
            QString shell = pwd->pw_shell;
            commandline << shell << "-ic"
                        << QString("%1; exec %2").arg(commandline_.join(' '), shell);
        } else {
            commandline << commandline_;
        }
        if (workingDir_.isNull())
            QProcess::startDetached(commandline.takeFirst(), commandline);
        else
            QProcess::startDetached(commandline.takeFirst(), commandline, workingDir_);
    }
private:
    QString text_;
    QStringList commandline_;
    QString workingDir_;
    bool shell_;
};



}
