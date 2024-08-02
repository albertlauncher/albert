// Copyright (c) 2022-2024 Manuel Schneider

#include "logging.h"
#include "terminalprovider.h"
#include "util.h"
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
static const char* CFG_CUSTOM_TERM = "custom_terminal";


class Terminal
{
public:
    virtual ~Terminal() = default;
    virtual const QString &name() const = 0;
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


#if defined(Q_OS_MAC)

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

#elif defined(Q_OS_UNIX)

class ExecutableTerminal : public Terminal
{

public:

    ExecutableTerminal(QString name, QStringList commandline)
        : name_(::move(name)), command_line_(::move(commandline)) {}

    const QString &name() const override { return name_; };

    const QStringList &commandLine() const { return command_line_; };

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

protected:

    QString name_;
    QStringList command_line_;

};


class CustomTerminal : public ExecutableTerminal
{
public:

    CustomTerminal():
        ExecutableTerminal(TerminalProvider::tr("Custom"),
                             settings()->value(CFG_CUSTOM_TERM, QStringList{}).toStringList()){}

    void setCommandLine(const QStringList &cmdline)
    {
        if (command_line_ != cmdline)
        {
            command_line_ = cmdline;
            settings()->setValue(CFG_CUSTOM_TERM, cmdline);
        }
    }
};

#endif


TerminalProvider::TerminalProvider():
    terminal_(nullptr)
{

#if defined(Q_OS_MAC)

    if (QFile::exists("/Applications/iTerm.app"))
        terminals_.emplace_back(make_unique<AppleScriptLaunchableTerminal>(
            "iTerm.app",
            R"(tell application "iTerm" to create window with default profile command "zsh -i %1")"
            ));

    if (QFile::exists("/System/Applications/Utilities/Terminal.app"))
        terminals_.emplace_back(make_unique<AppleScriptLaunchableTerminal>(
            "Terminal.app",
            R"(tell application "Terminal" to activate
               tell application "Terminal" to do script "exec zsh -i %1")"
            ));

#elif defined(Q_OS_UNIX)

    const vector<ExecutableTerminal> exec_terminals
    {
        {"Alacritty", {"alacritty", "-e"}},
        {"Black Box (Flatpak)", {"com.raggesilver.BlackBox", "--"}},
        {"Black Box", {"blackbox-terminal", "--"}},
        {"Black Box", {"blackbox", "--"}},
        {"Console", {"kgx", "-e"}},
        {"Contour (Flatpak)", {"org.contourterminal.Contour", "execute"}},
        {"Contour", {"contour", "execute"}},
        {"Cool Retro Term", {"cool-retro-term", "-e"}},
        {"Deepin Terminal", {"deepin-terminal", "-x"}},
        {"Elementary Terminal", {"io.elementary.terminal", "-x"}},
        {"Foot", {"foot"}},
        {"Gnome Terminal", {"gnome-terminal", "--"}},
        {"Kitty", {"kitty", "--"}},
        {"Konsole (Flatpak)", {"org.kde.konsole", "-e"}},
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
        {"Ptyxis (Flatpak)", {"app.devsuite.Ptyxis", "--"}},
        {"Ptyxis", {"ptyxis", "--"}},
        {"Terminology", {"terminology", "-e"}},
        {"Termite", {"termite", "-e"}},
        {"Tilix", {"tilix", "-e"}},
        {"UXTerm", {"uxterm", "-e"}},
        {"Urxvt", {"urxvt", "-e"}},
        {"WezTerm (Flatpak)", {"org.wezfurlong.wezterm", "-e"}},
        {"WezTerm", {"wezterm", "-e"}},
        {"XFCE-Terminal", {"xfce4-terminal", "-x"}},
        {"XTerm", {"xterm", "-e"}}
    };

    // Filter available supported terms by availability
    for (auto &t : exec_terminals)
        if (!QStandardPaths::findExecutable(t.commandLine()[0]).isNull())
            terminals_.emplace_back(make_unique<ExecutableTerminal>(t));

    // Add custom terminal
    auto ct = make_unique<CustomTerminal>();
    custom_terminal_ = ct.get();
    terminals_.emplace_back(::move(ct));

#endif

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

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)

void TerminalProvider::setCustomCommand(const QString &cmdln)
{
    custom_terminal_->setCommandLine(cmdln.split(QChar::Space));
}

QString TerminalProvider::customCommand() const
{
    return custom_terminal_->commandLine().join(QChar::Space);
}

#endif
