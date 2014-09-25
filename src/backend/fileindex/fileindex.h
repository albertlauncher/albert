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

#ifndef FILEINDEX_H
#define FILEINDEX_H

#include "abstractindexprovider.h"
#include "singleton.h"
#include "boost/filesystem.hpp"
#include <string>
#include <QMimeDatabase>

/**************************************************************************//**
 * @brief The FileIndex class
 */
class FileIndex : public AbstractIndexProvider, public Singleton<FileIndex>
{
	friend class Singleton<FileIndex>;

	FileIndex();
	~FileIndex(){}

public:
	class Item;

	void buildIndex() override;
	void saveIndex() const override;
	void loadIndex() override{}

	QMimeDatabase mimeDb;
};

/**************************************************************************//**
 * @brief The FileIndex::Item class
 */
class FileIndex::Item : public AbstractIndexProvider::Item
{
	friend class FileIndex;

public:
	Item(){}
	~Item(){}
	explicit Item(boost::filesystem::path p)
		: AbstractIndexProvider::Item(p.filename().string()), _path(p) {}

	inline std::string title()            const override {return _path.filename().string();}
	inline std::string complete()         const override {return _path.filename().string();}
	inline std::string infoText()         const override {return _path.string();}
	void               action(Action)           override;
	std::string        actionText(Action) const override;
	QIcon              icon()         const override;

protected:
	// Serialization
	friend class boost::serialization::access;
	template <typename Archive> void serialize(Archive &ar, const unsigned int version){
		ar & boost::serialization::base_object<AbstractIndexProvider::Item>(*this);
		std::string s;
		if(Archive::is_saving::value)
			s = _path.string();
		ar & s;
		if(Archive::is_loading::value)
			_path = s;
	}

	boost::filesystem::path _path;

	static const QMimeDatabase mimeDb;
};

#endif // FILEINDEX_H
