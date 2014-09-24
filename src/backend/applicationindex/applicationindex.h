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
#include "singleton.h"
#include <string>


/**************************************************************************//**
 * @brief The ApplicationIndex class
 */
class ApplicationIndex : public AbstractIndexProvider, public Singleton<ApplicationIndex>
{
	friend class Singleton<ApplicationIndex>;

	ApplicationIndex();
	~ApplicationIndex(){}

public:
	class Item;

	void buildIndex()      override;
	void saveIndex() const override;
	void loadIndex()       override{}
};

/**************************************************************************//**
 * @brief The ApplicationIndex::Item class
 */
class ApplicationIndex::Item : public AbstractIndexProvider::Item
{
	friend class ApplicationIndex;

public:
	Item(){}
	~Item(){}
	explicit Item(const std::string &name, const std::string &info, const std::string &iconName, const std::string &cmd, const bool term = false)
		: AbstractIndexProvider::Item(name), _info(info), _iconName(iconName), _exec(cmd), _term(term) {}

	inline std::string title()            const override {return _name;}
	inline std::string complete()         const override {return _name;}
	inline std::string infoText()         const override {return _info;}
	inline std::string iconName()         const override {return _iconName;}
	void               action(Action)           override;
	std::string        actionText(Action) const override;

protected:
	// Serialization
	friend class boost::serialization::access;
	template <typename Archive> void serialize(Archive &ar, const unsigned int version) {
	  ar & boost::serialization::base_object<AbstractIndexProvider::Item>(*this);
	  ar & _info;
	  ar & _iconName;
	  ar & _exec;
	  ar & _term;
	}

	std::string _info;
	std::string _iconName;
	std::string _exec;
	bool		_term;
};

#endif // APPLICATIONINDEX_H
