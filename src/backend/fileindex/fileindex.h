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
#include "boost/filesystem.hpp"
#include <string>

#ifdef FRONTEND_QT
#include <QMimeDatabase>
#endif

class FileIndex : public AbstractIndexProvider
{
public:
	class FileIndexItem : public AbstractIndexProvider::AbstractIndexItem
	{
	public:
		explicit FileIndexItem(){}
		explicit FileIndexItem(boost::filesystem::path p) : AbstractIndexItem(p.filename().string()), _path(p) {}
		~FileIndexItem(){}

		inline std::string title() const override {return _path.filename().string();}
		inline std::string complete() const override {return _path.filename().string();}
		inline std::string infoText() const override {return _path.string();}
		void               action(Action) override;
		std::string        actionText(Action) const override;
		std::string        iconName() const override;

	private:
		boost::filesystem::path _path;

		// Serialization
		friend class boost::serialization::access;
		template <typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & boost::serialization::base_object<AbstractIndexItem>(*this);
			std::string s;
			if(Archive::is_saving::value)
				s = _path.string();
			ar & s;
			if(Archive::is_loading::value)
				_path = s;
		}
	};

	static FileIndex* instance();

private:
	FileIndex();
	~FileIndex(){}
	void buildIndex() override;
	void saveIndex() const override;
	void loadIndex() override;

	static FileIndex *_instance;
//	magic_t _magic_cookie;
#ifdef FRONTEND_QT
	QMimeDatabase mimeDb;
#endif
};
#endif // FILEINDEX_H
