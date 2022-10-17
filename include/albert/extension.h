// Copyright (c) 2021 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>

namespace albert {
class ALBERT_EXPORT Extension  /// Interface for objects of the extension system
{
public:
    virtual ~Extension() {}
    virtual QString id() const = 0;  /// The guid of the extension
};
}

