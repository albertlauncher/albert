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

class Settings final : public QSettings,  public Singleton<Settings>
{
    friend class Singleton<Settings>;

public:
	~Settings(){}

private:
	Settings() : QSettings(QSettings::UserScope, "albert", "albert") {}
};
