// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <Qt>

namespace albert
{

/// Convention on the item roles passed around
enum class ALBERT_EXPORT ItemRoles
{
    TextRole = Qt::DisplayRole,  // QString
    SubTextRole = Qt::UserRole,  // QString
    InputActionRole,  // QString
    IconUrlsRole,  // QStringList, Urls for icon lookup
    ActionsListRole,  // QStringList, List of action names
};

}
