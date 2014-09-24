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

#include "albertengine.h"
#include <iostream>

//Modules
#include "websearch/websearch.h"
#include "fileindex/fileindex.h"
#include "calculator/calculator.h"
#include "bookmarkindex/bookmarkindex.h"
#include "applicationindex/applicationindex.h"
// DEBUG
#include <iostream>

/**********************************************************************//**
 * @brief AlbertEngine::AlbertEngine
 * @param parent
 */
AlbertEngine::AlbertEngine()
{
	_indizes.push_back(ApplicationIndex::instance());
	_indizes.push_back(FileIndex::instance());
	_indizes.push_back(BookmarkIndex::instance());

}

/**********************************************************************//**
 * @brief AlbertEngine::request
 * @param req
 * @return
 */
void AlbertEngine::query(const std::string &req, std::vector<AbstractServiceProvider::Item *> *res)
{
	Calculator::instance()->query(req, res);
	WebSearch::instance()->query(req, res);
	for (AbstractIndexProvider *i: _indizes)
		i->query(req, res);
	if (res->empty())
		WebSearch::instance()->queryAll(req, res);

	// Sort them by atime
	std::sort(res->begin(), res->end(), ATimeCompare());
}

/**********************************************************************//**
 * @brief AlbertEngine::buildIndex
 */
void AlbertEngine::buildIndex()
{
	for (AbstractIndexProvider *i: _indizes) {
		i->buildIndex();
	}
}

/**********************************************************************//**
 * @brief AlbertEngine::saveIndex
 */
void AlbertEngine::saveIndex() const
{
	for (AbstractIndexProvider *i : _indizes) {
		i->saveIndex();
	}
}
