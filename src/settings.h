#pragma once
#include <QSettings>

#define gSettings Settings::instance()

#define SETTINGS_CENTERED "showCentered"
#define SETTINGS_HOTKEY "hotkey"
#define SETTINGS_THEME "theme"
#define SETTINGS_PLGN_BLACKLIST "blacklist"

class Settings final : public QSettings
{
public:
	~Settings(){}
	static Settings *instance();

private:
	Settings() : QSettings(QSettings::UserScope, "albert", "albert") {}
	static Settings *_instance;
};
