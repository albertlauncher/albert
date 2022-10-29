// Copyright (c) 2022 Manuel Schneider

#include "logging.h"
#include "terminalprovider.h"
#include "albert/albert.h"
#include "albert/terminal.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <memory>
#include <pwd.h>
#include <unistd.h>
using namespace albert;
using namespace std;
static const char* CFG_TERM = "terminal";

#if defined(Q_OS_LINUX)
// TODO

#elif defined(Q_OS_MAC)
#endif

struct ExecutableTerminal : public albert::Terminal
{
    const char * name_;
    const char * command_;
    const char * script_option_;

    ExecutableTerminal(const char * name, const char * command, const char * script_option)
        : name_(name), command_(command), script_option_(script_option) {}

    ExecutableTerminal(const ExecutableTerminal&) = default;

    QString name() const override { return name_; };

    void openAt(const QString &working_dir) const override
    {
        albert::runDetachedProcess({command_}, working_dir);
    }

    void runCommand(const QStringList &command_line, const QString &working_dir) const override
    {
        albert::runDetachedProcess(QStringList{command_} << command_line, working_dir);
    };

    void runShellScript(const QString &script, Behavior closeBehavior, const QString &working_dir) const override
    {
        if (script.isEmpty())
            WARN << "Empty shell script";

        // Get the user shell (passwd must not be freed)
        passwd *pwd = getpwuid(geteuid());
        if (pwd == nullptr){
            CRIT << "Could not retrieve user shell. Terminal dysfunctional.";
            return;
        }

        QStringList commandline{command_, script_option_, pwd->pw_shell, "-ic"};
        switch (closeBehavior) {
            case Behavior::CloseOnExit:
                commandline << script;
                break;
            case Behavior::CloseOnSuccess:
                commandline << QString("%1 || exec %2").arg(script, pwd->pw_shell);
                break;
            case Behavior::NoClose:
                commandline << QString("%1; exec %2").arg(script, pwd->pw_shell);
                break;
        }

        albert::runDetachedProcess(commandline, working_dir);
    };
};


static const std::vector<ExecutableTerminal> exec_terminals
{
        // Distro terms
        {"Deepin Terminal", "deepin-terminal", "-x"},
        {"Elementary Terminal", "io.elementary.terminal", "-x"},
        {"Gnome Terminal", "gnome-terminal", "--"},
        {"Konsole", "konsole", "-e"},
        {"LXTerminal", "lxterminal", "-e"},
        {"Mate-Terminal", "mate-terminal", "-x"},
        {"XFCE-Terminal", "xfce4-terminal", "-x"},
        // others
        {"Cool Retro Term", "cool-retro-term", "-e"},
        {"QTerminal", "qterminal", "-e"},
        {"RoxTerm", "roxterm", "-x"},
        {"Terminator", "terminator", "-x"},
        {"Termite", "termite", "-e"},
        {"Tilix", "tilix", "-e"},
        {"UXTerm", "uxterm", "-e"},
        {"Urxvt", "urxvt", "-e"},
        {"XTerm", "xterm", "-e"}
};

TerminalProvider::TerminalProvider() : terminal_(nullptr)
{
    auto cfg_term_cmd = QSettings().value(CFG_TERM, QString()).toString();

    // Filter available supported terms by availability
    // Also set the configured terminal
    for (const auto & exec_term : exec_terminals){
        if (!QStandardPaths::findExecutable(exec_term.command_).isNull()){
            terminals_.emplace_back(std::make_unique<ExecutableTerminal>(exec_term));
            if (exec_term.command_ == cfg_term_cmd)
                terminal_ = terminals_.back().get();
        }
    }
    if (terminals_.empty())
        qFatal("No terminals available.");

    if (!terminal_){
        terminal_ = terminals_[0].get();
        WARN << "Configured terminal not available. Using:" << terminal_->name();
    }
}

const Terminal &TerminalProvider::terminal()
{
    return *terminal_;
}

const std::vector<std::unique_ptr<Terminal>> &TerminalProvider::availableTerminals() const
{
    return terminals_;
}

void TerminalProvider::setTerminal(uint i)
{
    terminal_ = terminals_[i].get();
}

QString TerminalProvider::name() const
{
    return terminal_->name();
}

void TerminalProvider::openAt(const QString &working_dir) const
{
    return terminal_->openAt(working_dir);
}

void TerminalProvider::runShellScript(const QString &script, Terminal::Behavior behavior, const QString &working_dir) const
{
    return terminal_->runShellScript(script, behavior, working_dir);
}

void TerminalProvider::runCommand(const QStringList &command_line, const QString &working_dir) const
{
    return terminal_->runCommand(command_line, working_dir);
}




