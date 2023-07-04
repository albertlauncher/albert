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
    IconRole = Qt::DecorationRole,  // QIcon for QWidgets
    SubTextRole = Qt::UserRole,  // QString
    IconPathRole,  // QString for QML
    IconUrlsRole,  // QStringList
    InputActionRole  // QString
};

}
