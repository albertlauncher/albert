// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <map>
#include <QString>
#include <locale>

class Settings
{
public:
	static Settings*          instance() noexcept;
	void                      load(QString path = relativeUserConfig);
	void                      save(QString path = relativeUserConfig) const;
	inline QString        get(QString key) const throw (std::out_of_range) {return _settings.at(key);}
	inline std::locale const& locale() const {return _locale;}
	inline QString        configDir() {return _homeDir + relativeUserConfigDir;}


private:
	Settings();
	~Settings() {}

	std::map<QString,QString> _settings;
	std::locale _locale;

	static Settings *_instance;
	QString _homeDir;
	static const QString systemConfig;
	static const QString relativeUserConfig;
	static const QString relativeUserConfigDir;

};

#endif // SETTINGS_H
