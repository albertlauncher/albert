// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QString>
#include "export.h"

namespace Core {

/**
 * @brief The action interface
 * A base class for actions (and items)
 */
class ALBERT_EXPORT Action
{
public:

    virtual ~Action() {}

    /** The description of the action */
    virtual QString text() const = 0;

    /** Activates the item */
    virtual void activate() const = 0;
};

}
