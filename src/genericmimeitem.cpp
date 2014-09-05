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

#include "genericmimeitem.h"
#include "unistd.h"

/**************************************************************************//**
 * @brief GenericMimeItem::action
 */
void GenericMimeItem::action(int nth)
{
	switch (nth) {
	case 1:
	{
		pid_t pid = fork();
		if (pid == 0) {
			// TODO SETID()
			execl("/usr/bin/xdg-open", "xdg-open", _uri.toStdString().c_str(), (char *)0);
			exit(1);
		}
	}
		break;
	default:
		break;
	}

}

/**************************************************************************//**
 * @brief GenericMimeItem::actionText
 * @return
 */
QString GenericMimeItem::actionText(int nth)
{
	switch (nth) {
	case 1:
//		return QString("Open %1").arg(_uri);
		return QString();
		break;
	default:
		return QString();
		break;
	}
}
