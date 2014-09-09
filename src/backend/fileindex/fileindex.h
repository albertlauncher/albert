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
#include <magic.h>

#ifdef FRONTEND_QT
#include <QMimeDatabase>
#endif

class FileIndex : public AbstractIndexProvider
{
public:
	class FileIndexItem : public AbstractIndexProvider::AbstractIndexItem
	{
	public:
		FileIndexItem() = delete;
		FileIndexItem(boost::filesystem::path p) : AbstractIndexItem(p.filename().string()), _path(p) {}
		~FileIndexItem(){}

		inline std::string complete() const override {return _path.filename().string();}
		inline std::string infoText() const override {return _path.string();}
		inline std::string uri() const override {return _path.string();}
		std::chrono::system_clock::time_point lastAccess() const override {return _lastAccess;}
		void               action(Action) override;
		std::string        actionText(Action) const override;
		std::string        iconName() const override;

	protected:
		boost::filesystem::path _path;
		std::chrono::system_clock::time_point _lastAccess;
	};

	static FileIndex* instance();

private:
	FileIndex();
	~FileIndex();

	void buildIndex() override;

	static FileIndex *_instance;
//	magic_t _magic_cookie;
#ifdef FRONTEND_QT
	QMimeDatabase mimeDb;
#endif
};
#endif // FILEINDEX_H
