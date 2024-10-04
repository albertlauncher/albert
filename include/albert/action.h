// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <functional>

namespace albert
{

///
/// Action used by result items (Item).
///
class ALBERT_EXPORT Action final
{
public:

    /// Action constructor
    /// \param id Identifier of the action.
    /// \param text Description of the action.
    /// \param function The action function.
    Action(QString id, QString text, std::function<void()> function) noexcept;

    /// The identifier of the action.
    QString id;

    /// The description of the action.
    QString text;

    /// The action function.
    std::function<void()> function;
};

}
