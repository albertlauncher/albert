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

#include "fileindexbuilder.h"
#include "fileindex.h"
#include "fileitem.h"
#include <QDebug>
#include <QDirIterator>

void FileIndexBuilder::run() {

	for(Service::Item *i : _result)
		delete i;
	_result.clear();

	qDebug() << "[FileIndex]\tLooking in: " << _ref->paths();

	for ( const QString &p : _ref->paths()) {
		QDirIterator it(p, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			if (it.fileInfo().isHidden() && !_ref->indexHiddenFiles())
				continue;
			_result.push_back(new FileIndex::Item(it.fileInfo()));
		}
	}
	qDebug() << "[FileIndex]\tFound " << _result.size() << " files.";
	emit fileIndexingDone();
}
