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

#ifndef ABSTRACTINDEXPROVIDER_H
#define ABSTRACTINDEXPROVIDER_H

#include "abstractserviceprovider.h"
#include <string>
#include <list>
#include <locale>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>


/**************************************************************************//**
 * @brief The AbstractIndexProvider class
 */
class AbstractIndexProvider : public AbstractServiceProvider
{
public:
	class Item;
	class CaseInsensitiveCompare;
	class CaseInsensitiveComparePrefix;

	AbstractIndexProvider(){}
	virtual ~AbstractIndexProvider(){}

	void query(const std::string&, std::vector<AbstractServiceProvider::Item*>*) override;
	virtual void buildIndex()      = 0;
	virtual void saveIndex() const = 0;
	virtual void loadIndex()       = 0;

protected:
	std::vector<Item*> _index;
	std::list<std::string> _watchPaths;
	std::string _indexFile;
};

/**************************************************************************//**
 * @brief The AbstractIndexProvider::Item class
 */
class AbstractIndexProvider::Item : public AbstractServiceProvider::Item
{
public:
	explicit Item(){}
	explicit Item(const std::string &name) : _name(name){}
	virtual ~Item(){}

	std::string _name;

protected:
	// Serialization
	friend class boost::serialization::access;
	template <typename Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar & _name;
		ar & _lastAccess;
	}
};

/**************************************************************************//**
 * @brief The AbstractIndexProvider::CaseInsensitiveCompare class
 */
class AbstractIndexProvider::CaseInsensitiveCompare
{
	std::ctype<char> const& myCType;
public:
	CaseInsensitiveCompare() = delete;
	CaseInsensitiveCompare(std::locale const& locale) : myCType( std::use_facet<std::ctype<char>>( locale ) ){}
	bool operator()( Item const* lhs, Item const* rhs ) const	{return (*this)( lhs->title(), rhs->title());}
	bool operator()( std::string const& lhs, Item const* rhs ) const {return (*this)( lhs, rhs->title() );}
	bool operator()( Item const* lhs, std::string const& rhs ) const {return (*this)( lhs->title(), rhs );}
	bool operator()( std::string const& lhs, std::string const& rhs ) const	{
		return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), *this);
	}
	bool operator()( char lhs, char rhs ) const	{return myCType.tolower(lhs) < myCType.tolower(rhs);}
};

/**************************************************************************//**
 * @brief The AbstractIndexProvider::CaseInsensitiveComparePrefix class
 */
class AbstractIndexProvider::CaseInsensitiveComparePrefix
{
	std::ctype<char> const& myCType;
public:
	CaseInsensitiveComparePrefix() = delete;
	CaseInsensitiveComparePrefix(std::locale const& locale) : myCType( std::use_facet<std::ctype<char>>( locale ) ){}
	bool operator()( Item const* pre, Item const* rhs ) const {return (*this)( pre->title(), rhs->title() );}
	bool operator()( std::string const& pre, Item const* rhs ) const {return (*this)( pre, rhs->title() );}
	bool operator()( Item const* pre, std::string const& rhs ) const {return (*this)( pre->title(), rhs );}
	bool operator()( std::string const& pre, std::string const& rhs ) const	{
		size_t m = std::min(pre.length(), rhs.length());
		return std::lexicographical_compare(pre.begin(), pre.begin()+m, rhs.begin(), rhs.begin()+m, *this);
	}
	bool operator()( char pre, char rhs ) const	{return myCType.tolower(pre) < myCType.tolower(rhs);}
};

#endif // ABSTRACTINDEXPROVIDER_H
