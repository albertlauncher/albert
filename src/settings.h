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
#define CFG_HOTKEY              "hotkey"
#define CFG_HOTKEY_DEF          ""
#define CFG_MAX_HISTORY         "maxHistory"
#define CFG_MAX_HISTORY_DEF     5
#define CFG_CENTERED            "showCentered"
#define CFG_CENTERED_DEF        true
#define CFG_THEME               "theme"
#define CFG_THEME_DEF           "Standard"
#define CFG_BLACKLISTED         "blacklisted"
#define CFG_BLACKLISTED_DEF     false
#define CFG_MAX_PROPOSALS       "itemCount"
#define CFG_MAX_PROPOSALS_DEF   5
#define SHOW_INFO               "showAnfo"
#define SHOW_INFO_DEF           true
#define SHOW_ACTION             "showAction"
#define SHOW_ACTION_DEF         true

class Settings final : public QSettings,  public Singleton<Settings>
{
    friend class Singleton<Settings>;

public:
	~Settings(){}

private:
	Settings() : QSettings(QSettings::UserScope, "albert", "albert") {}
};
