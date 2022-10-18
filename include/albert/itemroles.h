// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QAbstractItemModel>
#include <QString>
#include <array>
#include <Qt>

namespace albert {

/// Convention on the item roles passed around (ResultsModel, Frontends)
enum class ALBERT_EXPORT ItemRoles {
    TextRole = Qt::DisplayRole,
    IconRole = Qt::DecorationRole,
    SubTextRole = Qt::ToolTipRole,
    InputActionRole = Qt::UserRole, // Note this is used as int in QML
};

static const std::map<const ItemRoles, const QByteArray> QmlRoleNames {
        { ItemRoles::TextRole, "itemTextRole"},
        { ItemRoles::SubTextRole, "itemSubTextRole"},
        { ItemRoles::IconRole, "itemIconRole"},
        { ItemRoles::InputActionRole, "itemInputActionRole"},
};

}
