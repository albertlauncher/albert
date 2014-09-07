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
#include <QDesktopServices>
#include <QUrl>


void AbstractServiceProvider::AbstractItem::fallbackAction(AbstractServiceProvider::AbstractItem::Action) const
{
	QDesktopServices::openUrl(QUrl("https://www.google.de/search?q=" + _title));
//	pid_t pid = fork();
//	if (pid == 0) {
//		// TODO SETID()
//		execl("/usr/bin/chromium", "chromium", QString::fromLocal8Bit("https://www.google.de/search?q=%1").arg(_title).toStdString().c_str(), (char *)0);
//		exit(1);
//	}
}

QString AbstractServiceProvider::AbstractItem::fallbackActionText(AbstractServiceProvider::AbstractItem::Action) const
{
	return QString::fromLocal8Bit("Search for '%1' in web.").arg(_title);
}

