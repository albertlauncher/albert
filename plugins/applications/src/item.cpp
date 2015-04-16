// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "item.h"
#include "extension.h"

/** ***************************************************************************/
void AppInfo::action(const Query &q, Qt::KeyboardModifiers mods)
{
    ++_usage; _extension->action(*this, q, mods);
}

/** ***************************************************************************/
QString AppInfo::actionText(const Query &q, Qt::KeyboardModifiers mods) const
{
    return _extension->actionText(*this, q, mods);
}

/** ***************************************************************************/
QString AppInfo::titleText(const Query &q) const
{
    return _extension->titleText(*this, q);
}

/** ***************************************************************************/
QString AppInfo::infoText(const Query &q) const
{
    return _extension->infoText(*this, q);
}

/** ***************************************************************************/
const QIcon &AppInfo::icon() const
{
    return _extension->icon(*this);
}

/** ***************************************************************************/
uint AppInfo::usage() const
{
    return _usage;
}
