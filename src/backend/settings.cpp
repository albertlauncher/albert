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

#include "settings.h"
#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <pwd.h>
#include <unistd.h>


//REMOVE
#include <QDebug>

Settings* Settings::_instance= nullptr;
const std::string Settings::systemConfig = "/etc/albert/config";
const std::string Settings::relativeUserConfig= ".config/albert/config";
const std::string Settings::relativeUserConfigDir= ".config/albert/";



/**************************************************************************//**
 * @brief Settings::Settings
 */
Settings::Settings() : _locale(std::locale("")), _homeDir(getpwuid(getuid())->pw_dir)
{
	_homeDir.push_back('/');
}

/**************************************************************************//**
 * @brief Settings::load
 */
void Settings::load(std::string path)
{
	// Define a lambda
	std::function<void(const std::string &p)> loadSettings = [&] (const std::string &p)
	{
		std::ifstream file(p);
		if (!file.good()){
			std::cout << "[Settings] Config file not found:\t" << p << std::endl;
			return;
		}
		std::string str;
		while (std::getline(file, str))	{
			std::size_t found = str.find_first_of('=');
			if (found == std::string::npos || found == str.length())
				continue;
			_settings.insert(std::pair<std::string,std::string>(str.substr(0,found),str.substr(found+1)));
		}
		std::cout << "[Settings] Config file loaded:\t" << p << std::endl;
	};

	// Load global settings
	loadSettings(systemConfig);

	// Override with user settings
	std::string userSettings(_homeDir + path);
	loadSettings(userSettings);

	//DEBUG
//	std::cout << "[Settings]\t" << "[Key]\t\t[Value]"<< std::endl;
//	for ( std::pair<const std::string, std::string> &i : _settings)
//		std::cout << "[Settings]\t" << i.first << "\t" << i.second << std::endl;
}

/**************************************************************************//**
 * @brief Settings::save
 */
void Settings::save(std::string path) const
{
	std::ofstream file(path);
	if (!file.good())
		return;
	for (std::pair<std::string,std::string> i : _settings)
		file << i.first << "=" << i.second << std::endl;
}

/**************************************************************************//**
 * @brief Settings::instance
 * @return
 */
Settings *Settings::instance() noexcept
{
	if (_instance == nullptr)
		_instance = new Settings;
	return _instance;
}
