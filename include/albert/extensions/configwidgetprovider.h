// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../extension.h"
class QWidget;

namespace albert
{

/// Base class for all configurable plugins
class ALBERT_EXPORT ConfigWidgetProvider : virtual public Extension
{
public:
    /// Returns the cofig widget for this plugin
    /// @note The widgets objectName() is used as list item title
    /// @note Implementations must give away ownership (do not delete!)
    virtual QWidget* buildConfigWidget() = 0;
};

}
