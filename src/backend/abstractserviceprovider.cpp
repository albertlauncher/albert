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

#include "abstractserviceprovider.h"
#include <sstream>
#include <unistd.h>

void AbstractServiceProvider::AbstractItem::fallbackAction(AbstractServiceProvider::AbstractItem::Action) const
{
	std::string url("https://www.google.de/search?q=");
	url.append(_title);
	startDetached("xdg-open", url);
}

std::string AbstractServiceProvider::AbstractItem::fallbackActionText(AbstractServiceProvider::AbstractItem::Action) const
{
	std::ostringstream stringStream;
	stringStream << "Search for " << _title << " in the web.";
	return stringStream.str();
}

void AbstractServiceProvider::AbstractItem::startDetached(std::string cmd, std::string param)
{
	pid_t pid = fork();
	if (pid == 0) {
		pid_t sid = setsid();
		if (sid < 0) exit(EXIT_FAILURE);
		std::string cmd_p("/usr/bin/");
		cmd_p.append(cmd);
		execl(cmd_p.c_str(), cmd.c_str(), param.c_str(), (char *)0);
		exit(1);
	}
}

