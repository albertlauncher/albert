// Copyright (c) 2023 Manuel Schneider

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
    IconUrlsRole,  // Urls for icon lookup
};

}
