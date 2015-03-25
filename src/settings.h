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
#include <QSettings>
#include "singleton.h"

#define gSettings Settings::instance()
#define CFG_CENTERED "showCentered"
#define CFG_HOTKEY "hotkey"
#define CFG_THEME "theme"
#define CFG_PLGN_BLACKLIST "blacklist"
#define CFG_ITEMCOUNT "itemCount"
#define CFG_DEF_ITEMCOUNT 5
#define CONFIG_PATHS "AppIndex/Paths"

class Settings final : public QSettings,  public Singleton<Settings>
{
    friend class Singleton<Settings>;

public:
	~Settings(){}

private:
	Settings() : QSettings(QSettings::UserScope, "albert", "albert") {}
};
