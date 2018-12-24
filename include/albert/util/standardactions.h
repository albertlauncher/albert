// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QUrl>
#include <functional>
#include "albert/action.h"
#include "../core_globals.h"

namespace Core {


//! @brief Base class for standard actions
struct EXPORT_CORE StandardActionBase : public Action
{
public:
    StandardActionBase(const QString &text);
    QString text() const override;

private:
    QString text_;
};



//! @brief A standard action holding a std::function
struct EXPORT_CORE FuncAction : public StandardActionBase
{
public:
    FuncAction(const QString &text, std::function<void()> action);
    void activate() override;

private:
    std::function<void()> action_;
};



// A standard action that copies text into the clipboard
struct EXPORT_CORE ClipAction : public StandardActionBase
{
public:
    ClipAction(const QString &text, QString clipBoardText);
    void activate() override;

private:
    QString clipBoardText_;
};



// A standard action that opens an url using QDesktopServices
struct EXPORT_CORE UrlAction : public StandardActionBase
{
public:
    UrlAction(const QString &text, QUrl url);
    void activate() override;

private:
    QUrl url_ ;
};



// A standard action that starts a process
struct EXPORT_CORE ProcAction : public StandardActionBase
{
public:
    ProcAction(const QString &text, const QStringList &commandline, const QString &workingDirectory = QString());
    void activate() override;

protected:
    QStringList commandline_;
    QString workingDir_;
};



// A standard action that runs commands in a terminal
struct EXPORT_CORE TermAction : public ProcAction
{
    enum class CloseBehavior {
        CloseOnSuccess,
        CloseOnExit,
        DoNotClose
    };

public:

    /**
     * @brief TermAction constructor
     * @param text The description of the action
     * @param commandline The command to execute
     * @param workingDirectory The working directory where to run the command
     * @param shell Should the command be wrapped in a shell?
     * @param behavior The close behavior when using the shell
     */
    TermAction(const QString &text, const QStringList &commandline, const QString &workingDirectory = QString(),
               bool shell = true, CloseBehavior behavior = CloseBehavior::CloseOnSuccess);
    void activate() override;

private:
    bool shell_;
    CloseBehavior behavior_;
};



}
