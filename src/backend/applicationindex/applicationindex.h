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

#ifndef APPLICATIONINDEX_H
#define APPLICATIONINDEX_H

#include "abstractindexprovider.h"
#include <string>


class ApplicationIndex : public AbstractIndexProvider
{
public:
	class ApplicationIndexItem : public AbstractIndexProvider::AbstractIndexItem
	{
	public:
		ApplicationIndexItem() = delete;
		ApplicationIndexItem(std::string name,std::string info, std::string iconName, std::string cmd, bool term = false)
			: AbstractIndexItem(name), _info(info), _iconName(iconName), _exec(cmd), _term(term) {}
		~ApplicationIndexItem(){}

		inline std::string complete() const override {return _title;}
		inline std::string infoText() const override {return _info;}
		inline std::string uri()      const override {return "";}
		inline std::string iconName() const override {return _iconName;}
		std::chrono::system_clock::time_point lastAccess() const override {return _lastAccess;}
		void               action(Action) override;
		std::string        actionText(Action) const override;

	protected:
	std::chrono::system_clock::time_point _lastAccess;

		std::string _info;
		std::string _iconName;
		std::string _exec;
		bool		_term;
	};

	static ApplicationIndex* instance();

private:
	ApplicationIndex(){}
	~ApplicationIndex(){}

	void buildIndex() override;

	static ApplicationIndex *_instance;
};
#endif // APPLICATIONINDEX_H
