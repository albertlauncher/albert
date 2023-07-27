// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
#include <functional>

namespace albert
{

/// Action used by items.
class ALBERT_EXPORT Action final
{
public:

    /// Action constructor
    /// \param id Identifier of the action.
    /// \param text Description of the action.
    /// \param function The action function.
    Action(QString id, QString text, std::function<void()> function);

    QString id;  ///< Identifier of the action.
    QString text;  ///< Description of the action.
    std::function<void()> function;  ///< The action function.
};

}
