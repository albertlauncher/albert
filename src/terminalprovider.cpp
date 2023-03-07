// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "albert/util/util.h"
#include "terminalprovider.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryFile>
#include <memory>
#include <pwd.h>
#include <unistd.h>
using namespace albert;
using namespace std;
static const char* CFG_TERM = "terminal";


static QString userShell()
{
    // Get the user shell (passwd must not be freed)
    passwd *pwd = getpwuid(geteuid());
    if (pwd == nullptr){
        CRIT << "Could not retrieve user shell. Terminal dysfunctional.";
        return {};
    }
    return {pwd->pw_shell};
}


#if defined(Q_OS_LINUX)


struct ExecutableTerminal : public Terminal
{
    const char *name_;
    const vector<const char*> command_line_;

    ExecutableTerminal(const char* name, vector<const char*> commandline)
        : name_(name), command_line_(commandline.begin(), commandline.end()) {}

    QString name() const override { return name_; };

    void run(const QString &script, const QString &working_dir, bool close_on_exit) const override
    {
        QString shell = userShell();
        QStringList commandline{command_line_.begin(), command_line_.end()};
        commandline << shell;

        if (!script.isEmpty()) {
            if (close_on_exit)
                commandline << "-i" << "-c" << script;
            else
                commandline << "-i" << "-c" << QString("%1; exec %2").arg(script, shell);
        }

        albert::runDetachedProcess(commandline, working_dir);
    };
};


static const vector<ExecutableTerminal> exec_terminals
{
        {"Alacritty", {"alacritty", "-e"}},
        {"Black Box", {"blackbox", "-c"}},
        {"Console", {"kgx", "-e"}},
        {"Cool Retro Term", {"cool-retro-term", "-e"}},
        {"Deepin Terminal", {"deepin-terminal", "-x"}},
        {"Elementary Terminal", {"io.elementary.terminal", "-x"}},
        {"Foot", {"foot"}},
        {"Gnome Terminal", {"gnome-terminal", "--"}},
        {"Kitty", {"kitty", "--"}},
        {"Konsole", {"konsole", "-e"}},
        {"LXTerminal", {"lxterminal", "-e"}},
        {"Mate-Terminal", {"mate-terminal", "-x"}},
        {"QTerminal", {"qterminal", "-e"}},
        {"RoxTerm", {"roxterm", "-x"}},
        {"St", {"st", "-e"}},
        {"Terminator", {"terminator", "-u", "-g", "/dev/null", "-x"}},  // TODO remove in future. See #1177 and https://github.com/gnome-terminator/terminator/issues/702
        {"Terminology", {"terminology", "-e"}},
        {"Termite", {"termite", "-e"}},
        {"Tilix", {"tilix", "-e"}},
        {"UXTerm", {"uxterm", "-e"}},
        {"Urxvt", {"urxvt", "-e"}},
        {"WezTerm", {"wezterm", "cli", "spawn", "--"}},
        {"XFCE-Terminal", {"xfce4-terminal", "-x"}},
        {"XTerm", {"xterm", "-e"}}
};


static vector<unique_ptr<Terminal>> findTerminals()
{
    vector<unique_ptr<Terminal>> result;
    // Filter available supported terms by availability
    for (const auto & exec_term : exec_terminals)
        if (!QStandardPaths::findExecutable(exec_term.command_line_[0]).isNull())
            result.emplace_back(make_unique<ExecutableTerminal>(exec_term));
    return result;
}


#elif defined(Q_OS_MAC)


static void execAppleScript(const QString &script)
{
    QProcess p;
    DEBG << "Execute AppleScript: " << script;
    p.start("/usr/bin/osascript", {"-l", "AppleScript"});
    p.waitForStarted();
    p.write(script.toUtf8());
    p.closeWriteChannel();
    p.waitForFinished();
    if (p.exitCode())
        WARN << "Executed AppleScript " << p.exitCode() << p.error();
}


static QString writeCommandFile(const QString &script, bool close_on_exit, const QString &working_dir)
{
    auto file = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).filePath("terminal.command");
    QFile f(file);
    if (f.open(QIODevice::WriteOnly)) {
        if (!working_dir.isEmpty())
            f.write(QString("cd \"%1\";").arg(working_dir).toUtf8());
        f.write("clear;");
        f.write(script.toUtf8());
        if (!close_on_exit)
            // space needed because exec behaves differently on ;;
            f.write(QString(" ; exec %1 -i").arg(userShell()).toUtf8());
        f.close();
    }
    return file;
}


struct iTerm : public Terminal
{
    QString name() const override { return "iTerm.app"; };
    void run(const QString &script, const QString &working_dir, bool close_on_exit) const override
    {
        auto file = writeCommandFile(script, close_on_exit, working_dir);
        execAppleScript(QString("tell application \"iTerm2\"\n"
                                "create window with default profile command \"zsh -i %1\"\n"
                                "end tell").arg(file));
    };
};


struct AppleTerminal : public Terminal
{
    QString name() const override { return "Terminal.app"; };
    void run(const QString &script, const QString &working_dir, bool close_on_exit) const override
    {
        auto file = writeCommandFile(script, close_on_exit, working_dir);
        execAppleScript(QString("tell application \"Terminal\"\n"
                                "activate\n"
                                "do script \"exec zsh -i %1\"\n"
                                "end tell").arg(file));
    };
};


static vector<unique_ptr<Terminal>> findTerminals()
{
    vector<unique_ptr<Terminal>> result;

    if (QFile::exists("/Applications/iTerm.app"))
        result.emplace_back(make_unique<iTerm>());

    if (QFile::exists("/System/Applications/Utilities/Terminal.app"))
        result.emplace_back(make_unique<AppleTerminal>());

    return result;
}


#endif


TerminalProvider::TerminalProvider() : terminal_(nullptr)
{
    terminals_ = findTerminals();
    if (terminals_.empty())
        qFatal("No terminals available.");

    // Set the configured terminal
    auto cfg_term_cmd = QSettings(qApp->applicationName()).value(CFG_TERM, QString()).toString();
    for (const auto & terminal : terminals_)
        if (terminal->name() == cfg_term_cmd)
            terminal_ = terminal.get();

    if (!terminal_){
        terminal_ = terminals_[0].get();
        INFO << "Configured terminal not available. Using:" << terminal_->name();
        setTerminal(0);
    }
}

const Terminal &TerminalProvider::terminal()
{
    return *terminal_;
}

const vector<unique_ptr<Terminal>> &TerminalProvider::terminals() const
{
    return terminals_;
}

void TerminalProvider::setTerminal(uint i)
{
    terminal_ = terminals_[i].get();
    QSettings(qApp->applicationName()).setValue(CFG_TERM, terminal_->name());
}
