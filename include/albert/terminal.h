// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>

namespace albert
{
class Terminal
{
public:
    virtual ~Terminal() {}

    enum Behavior
    {
        CloseOnSuccess,
        CloseOnExit,
        NoClose
    };

    virtual QString name() const = 0;

    virtual void openAt(const QString &working_dir = {}) const = 0;

    virtual void runCommand(const QStringList &command_line, const QString &working_dir = {}) const = 0;

    virtual void runShellScript(const QString &script, Behavior behavior, const QString &working_dir = {}) const = 0;
};

}