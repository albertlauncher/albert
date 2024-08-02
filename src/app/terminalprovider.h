// Copyright (c) 2021-2024 Manuel Schneider

#pragma once
#include <QCoreApplication>
#include <QString>
class Terminal;
class CustomTerminal;

class TerminalProvider
{
    Q_DECLARE_TR_FUNCTIONS(Terminals)

public:

    TerminalProvider();
    ~TerminalProvider();

    QString name() const;

    void run(const QString &script,
             const QString &working_dir,
             bool close_on_exit) const;

    QStringList terminals() const;
    void setTerminal(uint);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    void setCustomCommand(const QString&);
    QString customCommand() const;
#endif

private:

    std::vector<std::unique_ptr<Terminal>> terminals_;
    Terminal *terminal_;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    CustomTerminal *custom_terminal_;
#endif

};
