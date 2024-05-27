// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include <QString>

class Terminal
{
public:
    virtual ~Terminal() = default;
    virtual QString name() const = 0;
    virtual void run(const QString &script, const QString &working_dir, bool close_on_exit) const = 0;
};

class TerminalProvider
{
public:
    TerminalProvider();

    const Terminal &terminal();

    const std::vector<std::unique_ptr<Terminal>> &terminals() const;
    void setTerminal(uint);

private:
    std::vector<std::unique_ptr<Terminal>> terminals_;
    Terminal *terminal_;
};
