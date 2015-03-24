#pragma once
#include <QSettings>
#include "singleton.h"

#define gSettings Settings::instance()

#define SETTINGS_CENTERED "showCentered"
#define SETTINGS_HOTKEY "hotkey"
#define SETTINGS_THEME "theme"
#define SETTINGS_PLGN_BLACKLIST "blacklist"

class Settings final : public QSettings,  public Singleton<Settings>
{
    friend class Singleton<Settings>;

public:
	~Settings(){}

private:
	Settings() : QSettings(QSettings::UserScope, "albert", "albert") {}
};
