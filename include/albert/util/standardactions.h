// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QUrl>
#include <functional>
#include "albert/action.h"
#include "../core_globals.h"

namespace Core {


//! @brief Base class for standard actions implementing the text getter
struct EXPORT_CORE StandardActionBase : public Action
{
    /**
     * @param text The description of the action
     */
    explicit StandardActionBase(const QString &text);
    QString text() const override;
private:
    const QString text_;
};


//! @brief Runs a function
struct EXPORT_CORE FuncAction : public StandardActionBase
{
    /**
     * @param text The description of the action
     * @param action The fuction to execute
     */
    explicit FuncAction(const QString &text, std::function<void()> action);
    void activate() override;
private:
    const std::function<void()> action_;
};


//! @brief Copies text into the clipboard
struct EXPORT_CORE ClipAction : public StandardActionBase
{
    /**
     * @param text The description of the action
     * @param clipBoardText The text to put in the clipboard
     */
    explicit ClipAction(const QString &text, QString clipBoardText);
    void activate() override;
private:
    const QString clipBoardText_;
};


//! @brief Opens an URL using the system scheme/mime hanlders
struct EXPORT_CORE UrlAction : public StandardActionBase
{
    /**
     * @param text The description of the action
     * @param commandline The URL to open with the correspondig handler
     */
    explicit UrlAction(const QString &text, QUrl url);
    void activate() override;
private:
    const QUrl url_;
};


//! @brief Starts a detached process
struct EXPORT_CORE ProcAction : public StandardActionBase
{
    /**
     * @param text The description of the action
     * @param commandline The program with arguments to execute
     * @param workingDirectory The working directory
     */
    explicit ProcAction(const QString &text, const QStringList &commandline, const QString &workingDirectory = QString());
    void activate() override;
protected:
    const QStringList commandline_;
    const QString workingDir_;
};


//! @brief Starts a commandline in a detached user definded terminal
struct EXPORT_CORE TermAction : public StandardActionBase
{
    /**
     * @param text The description of the action
     * @param commandline The program with arguments to execute
     * @param workingDirectory The working directory
     */
    explicit TermAction(const QString &text, const QStringList &commandline, const QString &workingDirectory = QString());
    void activate() override;
protected:
    const QStringList commandline_;
    const QString workingDir_;
};


//! @brief Starts a script wrapped in the user shell in a detached user definded terminal
struct EXPORT_CORE ShTermAction : public StandardActionBase
{
    enum CloseBehavior {
        CloseOnSuccess,
        CloseOnExit,
        DoNotClose
    };

    /**
     * @param text The description of the action
     * @param script The shell script to execute
     * @param closeBehavior What happens when the script finished
     * @param workingDirectory The working directory
     */
    explicit ShTermAction(const QString &text, const QString &script, CloseBehavior closeBehavior = CloseOnSuccess, const QString &workingDirectory = QString());
    void activate() override;
protected:
    const QString script_;
    const CloseBehavior closeBehavior_;
    const QString workingDir_;
};

}
