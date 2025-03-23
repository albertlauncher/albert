// SPDX-FileCopyrightText: 2025 Manuel Schneider
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
    /// \param id \copybrief id
    /// \param text \copybrief text
    /// \param function \copybrief function
    /// \param hideOnActivation \copybrief hide_on_activation
    Action(QString id, QString text, std::function<void()> function, bool hideOnActivation = true) noexcept;

    /// The identifier.
    QString id;

    /// The description.
    QString text;

    /// The action.
    std::function<void()> function;

    /// The activation behavior.
    bool hide_on_activation;
};

}
