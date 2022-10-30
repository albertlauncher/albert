// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include <QString>

class Terminal
{
public:
    virtual ~Terminal() {}
    virtual QString name() const = 0;
    virtual void run(const QString &script, const QString &working_dir, bool close_on_exit) const = 0;
};

class TerminalProvider : public Terminal
{
public:
    TerminalProvider();

    const Terminal &terminal();

    const std::vector<std::unique_ptr<Terminal>> &terminals() const;
    void setTerminal(uint);

    QString name() const override;
    void run(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false) const override;


private:
    std::vector<std::unique_ptr<Terminal>> terminals_;
    Terminal *terminal_;
};
