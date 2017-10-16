// Copyright (C) 2014-2017 Manuel Schneider

#pragma once
#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QDesktopServices>
#include <QStringList>
#include <QProcess>
#include <functional>
#include "core/action.h"
#include "util/shutil.h"
#include "core_globals.h"

extern QString terminalCommand;

namespace Core {


//! @brief A standard action holding a std::function
struct EXPORT_CORE FuncAction : public Action
{
public:
    template<class S1 = QString, class S2 = std::function<void()>>
    FuncAction(S1&& text = QString(), S2&& action = std::function<void()>())
        : text_(std::forward<S1>(text)), action_(std::forward<S2>(action)) { }
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
    template<class S1 = QString, class S2 = QString>
    ClipAction(S1&& text = QString(), S2&& clipBoardText = QString())
        : text_(std::forward<S1>(text)), clipBoardText_(std::forward<S2>(clipBoardText)) { }
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
    template<class S1 = QString, class S2 = QString>
    UrlAction(S1&& text = QString(), S2&& url = QUrl())
        : text_(std::forward<S1>(text)), url_(std::forward<S2>(url)) { }
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
    template<class S1 = QString,
             class S2 = QStringList,
             class S3 = QString>
    ProcAction(S1&& text = QString(),
               S2&& commandline = QStringList(),
               S3&& workingDirectory = QString())
        : text_(std::forward<S1>(text)),
          commandline_(std::forward<S2>(commandline)),
          workingDir_(std::forward<S3>(workingDirectory)) { }
    QString text() const override { return text_; }
    void activate() override {
        if (commandline_.isEmpty())
            return;
        QStringList tmp = commandline_;
        QProcess::startDetached(tmp.takeFirst(), tmp, workingDir_);
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
    template<class S1 = QString,
             class S2 = QStringList,
             class S3 = QString>
    TermAction(S1&& text = QString(),
               S2&& commandline = QStringList(),
               S3&& workingDirectory = QString())
        : text_(std::forward<S1>(text)),
          commandline_(std::forward<S2>(commandline)),
          workingDir_(std::forward<S3>(workingDirectory)) { }
    QString text() const override { return text_; }
    void activate() override {
        QStringList arguments = Core::ShUtil::split(terminalCommand);
        arguments.append(commandline_);
        QString command = arguments.takeFirst();
        if (workingDir_.isNull())
            QProcess::startDetached(command, arguments);
        else
            QProcess::startDetached(command, arguments, workingDir_);
    }
private:
    QString text_;
    QStringList commandline_;
    QString workingDir_;
};



}
