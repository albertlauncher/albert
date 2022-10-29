// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "export.h"
#include "albert/terminal.h"
#include <QString>

class TerminalProvider : public albert::Terminal
{
public:
    TerminalProvider();

    const Terminal &terminal();

    const std::vector<std::unique_ptr<Terminal>> &availableTerminals() const;
    void setTerminal(uint);

    QString name() const override;
    void openAt(const QString &working_dir = {}) const override;
    void runCommand(const QStringList &command_line, const QString &working_dir = {}) const override;
    void runShellScript(const QString &script, Behavior behavior, const QString &working_dir = {}) const override;


private:
    std::vector<std::unique_ptr<Terminal>> terminals_;
    Terminal *terminal_;
};
