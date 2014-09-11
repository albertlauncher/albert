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

#ifndef WEBSEARCH_H
#define WEBSEARCH_H

#include "abstractserviceprovider.h"
#include <string>
#include <vector>


//TODO IN SRC
#include <unistd.h>

class WebSearch : public AbstractServiceProvider
{
public:
	class WebSearchItem : public AbstractServiceProvider::AbstractItem
	{
	public:
		WebSearchItem() = delete;
		WebSearchItem( const std::string &name, const std::string &url, const std::string &sc, const std::string &iconName)
			: _name(name), _url(url), _shortcut(sc), _iconName(iconName) {}
		~WebSearchItem(){}

		inline std::string title() const override {return "Search '" + ((_searchTerm.empty())?"...":_searchTerm) + "' in " + _name;}
		inline std::string complete() const override {return _name + " " + _searchTerm;}
		inline std::string infoText() const override {return std::string(_url).replace(_url.find("%s"), 2, _searchTerm);}
		void               action(Action) override;
		std::string        actionText(Action) const override;
		std::string        iconName() const override;

		std::string        shortcut() const {return _shortcut;}
		std::string        name() const {return _name;}
		std::string        searchTerm() const {return _searchTerm;}

		void setTerm(const std::string &term) {_searchTerm = term;}

	protected:
		std::string _searchTerm;
		const std::string _name;
		const std::string _url;
		const std::string _shortcut;
		const std::string _iconName;
	};

	static WebSearch* instance();
	void        query(const std::string&, std::vector<AbstractItem*>*) override;
	void        queryAll(const std::string&, std::vector<AbstractItem*>*);
	void        defaultSearch(const std::string& term) const;
	std::string defaultSearchText(const std::string& term) const;

private:
	WebSearch();
	~WebSearch();

	std::vector<WebSearchItem*> _searchEngines;
	static WebSearch *_instance;

};
#endif // WEBSEARCH_H
