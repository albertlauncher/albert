// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/extension.h>
#include <albert/item.h>
#include <memory>
#include <vector>

namespace albert
{
class Item;

///
/// Abstract fallback item provider.
///
/// \ingroup core
///
class ALBERT_EXPORT FallbackHandler : virtual public Extension
{
public:

    ///
    /// Returns fallback items for _query_.
    ///
    virtual std::vector<std::shared_ptr<Item>> fallbacks(const QString &) const = 0;

protected:

    ///
    /// Destructs the fallback handler.
    ///
    ~FallbackHandler() override;

};

}
