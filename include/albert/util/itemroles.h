// Copyright (C) 2014-2019 Manuel Schneider

#pragma once
#include  <Qt>

namespace Core {

enum ItemRoles {
    TextRole = 0,
    ToolTipRole,
    DecorationRole,
    CompletionRole = Qt::UserRole, // Note this is used as int in QML
    ActionRole,
    AltActionRole,
    FallbackRole
};

}
