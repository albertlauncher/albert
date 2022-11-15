// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../extension.h"
class QWidget;

namespace albert
{

enum class ConfigGroup {
    General,
    Extension
};
/// Base class for all configurable plugins
class ALBERT_EXPORT ConfigWidgetProvider : virtual public Extension
{
public:

    virtual QWidget* buildConfigWidget() = 0;  /// Config widget factory. Gives away ownership.
    virtual QString configTitle() const = 0;  /// Config tree item title
    virtual ConfigGroup configGroup() const;  /// Config tree item group
};

}
