// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/configwidgetprovider.h"
using namespace albert;

ConfigGroup ConfigWidgetProvider::configGroup() const
{
    return ConfigGroup::Extension;
}
