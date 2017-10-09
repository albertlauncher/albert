// Copyright (C) 2014-2017 Manuel Schneider

#pragma once
#include <QString>
#include <functional>
#include "core_globals.h"

namespace Core {

/**
 * @brief The albert action type
 */
class EXPORT_CORE Action
{
public:

    Action() {}

    template<class QString, class Function>
    Action(QString&& text, Function&& function)
        : text(std::forward<QString>(text)),
          function(std::forward<Function>(function)) { }

    /** The description of the action*/
    QString text;

    /** The action */
    std::function<void()> function;

    /** Convenience function. Executes the action */
    void activate() { function(); }
};

}
