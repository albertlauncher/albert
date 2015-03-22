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

#ifndef WEBSEARCHITEM_H
#define WEBSEARCHITEM_H

#include "websearch.h"
#include <QString>
#include <QIcon>

class WebSearch::Item : public Service::Item
{
	friend class WebSearchWidget;
	friend class WebSearch;

public:
	Item(){_lastAccess = 1;}
	~Item(){}

	inline QString title() const override {return "Search '" + ((_searchTerm.isEmpty())?"...":_searchTerm) + "' in " + _name;}
	inline QString complete() const override {return _name + " " + _searchTerm;}
	inline QString infoText() const override {return QString(_url).replace("%s", _searchTerm);}
	QIcon icon() const override;

	void    action() override;
	QString actionText() const override;
	void    altAction() override;
	QString altActionText() const override;

	QString shortcut() const {return _shortcut;}
	QString name() const {return _name;}
	QString searchTerm() const {return _searchTerm;}

protected:
	QString _searchTerm;
	QString _name;
	QString _url;
	QString _shortcut;
	QString _iconPath;

	// Serialization
	void serialize (QDataStream &out) const override;
	void deserialize (QDataStream &in) override;
};
#endif // WEBSEARCHITEM_H
