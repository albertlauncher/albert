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

/** ***************************************************************************/
class InformationalItem final : public ItemInterface // ToDO This is alway shown last , twwak relevance
{
public:
    ItemInterface() {}
    virtual ~ItemInterface(){}

    inline QString     actionText(const Query &, Qt::KeyboardModifiers) const override { return _actionText; }
    inline void        setActionText(const QString &param) { _actionText = param; }

    inline QString     titleText (const Query &) const override { return _titleText; }
    inline void        setTitleText(const QString &param) { _titleText = param; }

    inline QString     infoText  (const Query &) const override { return _infoText; }
    inline void        setInfoText(const QString &param) { _infoText = param; }

    inline const QIcon &icon     () const override { return _icon; }
    inline void        setIcon(const QIcon &param) { _icon = param; }

    inline uint        usage     () const override { return UINT32_MAX;}

private:
    QString _actionText;
    QString _titleText;
    QString _infoText;
    QString _icon;
};
