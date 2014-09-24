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

#ifndef ABSTRACTSERVICEPROVIDER_H
#define ABSTRACTSERVICEPROVIDER_H

#include <string>
#include <vector>
#include <chrono>

class AbstractServiceProvider
{
public:
	class AbstractItem
	{
	public:
		enum class Action { Enter, Alt, Ctrl };

		explicit AbstractItem(): _lastAccess(0) {}
		virtual ~AbstractItem(){}

		virtual std::string title() const = 0;
		virtual std::string iconName() const = 0;
		virtual std::string complete() const = 0;
		virtual void        action(Action) = 0;
		virtual std::string actionText(Action) const = 0;
		virtual std::string infoText() const = 0;
		inline  u_int64_t    lastAccess() const {return _lastAccess;}

		u_int64_t _lastAccess; // secs since epoch
	};

	AbstractServiceProvider(){}
	virtual ~AbstractServiceProvider(){}
	virtual void query(const std::string&, std::vector<AbstractItem*>*) = 0;
};

#endif // ABSTRACTSERVICEPROVIDER_H
