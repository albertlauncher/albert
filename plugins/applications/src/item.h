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

#pragma once
#include "plugininterfaces/iteminterface.h"
class Extension;

/** ***************************************************************************/
class AppInfo final : public ItemInterface
{
    friend class Extension;

public:
    AppInfo() = delete;
    explicit AppInfo(Extension *ext) : _extension(ext) {}
    ~AppInfo(){}

    void         action    (const Query &q, Qt::KeyboardModifiers mods) override;
    QString      actionText(const Query &q, Qt::KeyboardModifiers mods) const override;
    QString      titleText (const Query &q) const override;
    QString      infoText  (const Query &q) const override;
    const QIcon  &icon     () const override;
    uint         usage     () const override;

private:
    QString    _path;
    QString    _name;
    QString    _altName;
    QString    _exec;
    QIcon      _icon;
    uint       _usage;
    Extension* _extension; // Should never be invalid since the extension must not unload
};
