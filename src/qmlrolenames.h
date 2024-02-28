// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/frontend/itemroles.h"
#include <QHash>

namespace albert
{
QHash<int, QByteArray> QmlRoleNames = {
    {(int)ItemRoles::TextRole, "itemText"},
    {(int)ItemRoles::SubTextRole, "itemSubText"},
    {(int)ItemRoles::InputActionRole, "itemInputAction"},
    {(int)ItemRoles::IconUrlsRole, "itemIconUrls"},
    {(int)ItemRoles::ActionsListRole, "itemActionsList"}
};
}
