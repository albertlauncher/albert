// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/logging.h"
#include "albert/util.h"
#include "terminalprovider.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
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


class Terminal
{
public:
    virtual ~Terminal() = default;
    virtual QString name() const = 0;
    virtual void run(const QString &script, const QString &working_dir, bool close_on_exit) const = 0;
};


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


#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)

class ExecutableTerminal : public Terminal
{
public:
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
    {"Black Box", {"blackbox-terminal", "--"}},
    {"Console", {"kgx", "-e"}},
    {"Contour", {"contour", "execute"}},
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
    // TODO remove in future. Like in 2027 🤷
    // See #1177 and https://github.com/gnome-terminator/terminator/issues/702 and 660
    {"Terminator (<=2.1.2)", {"terminator", "-u", "-g", "/dev/null", "-x"}},
    {"Terminator", {"terminator", "-u", "-x"}},
    {"Terminology", {"terminology", "-e"}},
    {"Termite", {"termite", "-e"}},
    {"Tilix", {"tilix", "-e"}},
    {"UXTerm", {"uxterm", "-e"}},
    {"Urxvt", {"urxvt", "-e"}},
    {"WezTerm", {"wezterm", "cli", "spawn", "--"}},
    {"XFCE-Terminal", {"xfce4-terminal", "-x"}},
    {"XTerm", {"xterm", "-e"}}
};

#elif defined(Q_OS_MAC)


class AppleScriptLaunchableTerminal : public Terminal
{

public:
    const char *name_;
    const char *apple_script_;

    /// \brief AppleScriptLaunchableTerminal
    /// \param name the name of the terminal
    /// \param apple_script the apple script to launch the terminal
    /// \note the apple script must contain the placeholder %1 for the command file
    AppleScriptLaunchableTerminal(const char* name, const char* apple_script)
        : name_(name), apple_script_(apple_script) {}

    QString name() const override { return name_; };

    void run(const QString &script, const QString &working_dir, bool close_on_exit) const override
    {
        // Note for future self
        // QTemporaryFile does not start
        // Deleting the file introduces race condition

        if (QFile file(QDir(cacheLocation()).filePath("terminal_command"));
            file.open(QIODevice::WriteOnly))
        {
            if (!working_dir.isEmpty())
                file.write(QString(R"(cd "%1";)").arg(working_dir).toUtf8());
            file.write("clear;");
            file.write(script.toUtf8());
            if (!close_on_exit)
                // space needed because exec behaves differently on ;;
                file.write(QString(" ; exec %1 -i").arg(userShell()).toUtf8());
            file.close();

            albert::runDetachedProcess({"/usr/bin/osascript", "-l", "AppleScript",
                                        "-e", QString(apple_script_).arg(file.fileName())});
        }
        else
            WARN << QString("Running command in %1 failed. Could not create temporary file: %2")
                    .arg(name(), file.errorString());
    }
};

#endif

static vector<unique_ptr<Terminal>> findTerminals()
{
    vector<unique_ptr<Terminal>> result;

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)

    // Filter available supported terms by availability
    for (const auto & exec_term : exec_terminals)
        if (!QStandardPaths::findExecutable(exec_term.command_line_[0]).isNull())
            result.emplace_back(make_unique<ExecutableTerminal>(exec_term));

#elif defined(Q_OS_MAC)

    if (QFile::exists("/Applications/iTerm.app"))
        result.emplace_back(make_unique<AppleScriptLaunchableTerminal>(
            "iTerm.app",
            R"(tell application "iTerm" to create window with default profile command "zsh -i %1")"
            ));

    if (QFile::exists("/System/Applications/Utilities/Terminal.app"))
        result.emplace_back(make_unique<AppleScriptLaunchableTerminal>(
            "Terminal.app",
            R"(tell application "Terminal" to activate
               tell application "Terminal" to do script "exec zsh -i %1")"
            ));

#endif

    sort(result.begin(), result.end(),
         [](const auto &a, const auto &b)
         { return a->name() < b->name(); });

    return result;
}


TerminalProvider::TerminalProvider():
    terminals_(findTerminals()),
    terminal_(nullptr)
{
    if (terminals_.empty())
    {
        CRIT << "No terminals available.";
        return;
    }

    if (!settings()->contains(CFG_TERM))
    {
        // if unconfigured, set the first terminal as default
        terminal_ = terminals_[0].get();
    }
    else
    {
        // if configured, set the configured terminal
        auto name = settings()->value(CFG_TERM).toString();
        for (const auto & terminal : terminals_)
            if (terminal->name() == name)
                terminal_ = terminal.get();

        // if not found, use the first terminal
        if (!terminal_){
            terminal_ = terminals_[0].get();
            INFO << QString("Configured terminal '%1' not available. Using: '%2'")
                        .arg(name, terminal_->name());
        }
    }
}

TerminalProvider::~TerminalProvider() = default;

QString TerminalProvider::name() const
{
    return terminal_ ? terminal_->name() : QStringLiteral("N/A");
}

void TerminalProvider::run(const QString &script, const QString &working_dir, bool close_on_exit) const
{
    if (terminal_)
        terminal_->run(script, working_dir, close_on_exit);
    else
    {
        auto m = QT_TR_NOOP("Failed to run command. No terminal available.");
        WARN << m;
        QMessageBox::critical(nullptr, qApp->applicationName(), tr(m));
    }
}

QStringList TerminalProvider::terminals() const
{
    QStringList names;
    for (const auto & terminal : terminals_)
        names << terminal->name();
    return names;
}

void TerminalProvider::setTerminal(uint i)
{
    terminal_ = terminals_.at(i).get();
    albert::settings()->setValue(CFG_TERM, terminal_->name());
}
