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

#ifndef BOOKMARKINDEX_H
#define BOOKMARKINDEX_H

#include "abstractindexprovider.h"
#include "singleton.h"
#include "boost/filesystem.hpp"
#include <string>
#include <iostream>


/**************************************************************************//**
 * @brief The BookmarkIndex class
 */
class BookmarkIndex : public AbstractIndexProvider, public Singleton<BookmarkIndex>
{
	friend class Singleton<BookmarkIndex>;

	BookmarkIndex();
	~BookmarkIndex(){}

public:
	class Item;

	void buildIndex()      override;
	void saveIndex() const override;
	void loadIndex()       override{}
};

/**************************************************************************//**
 * @brief The BookmarkIndex::Item class
 */
class BookmarkIndex::Item : public AbstractIndexProvider::Item
{
	friend class BookmarkIndex;

public:
	Item(){}
	~Item(){}
	explicit Item(const std::string &name, const std::string &url)
		: AbstractIndexProvider::Item(name), _url(url){}

	inline std::string title() const override {return _name;}
	inline std::string complete() const override {return _name;}
	inline std::string infoText() const override {return _url;}
	void               action(Action) override;
	std::string        actionText(Action) const override;
	QIcon              icon() const override;


protected:
	// Serialization
	friend class boost::serialization::access;
	template <typename Archive> void serialize(Archive &ar, const unsigned int version){
	  ar & boost::serialization::base_object<AbstractIndexProvider::Item>(*this);
	  ar & _url;
	}

	std::string _url;
};

#endif // BOOKMARKINDEX_H
