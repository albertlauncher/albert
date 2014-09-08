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
#include "filesystemindex/filesystemindex.h"

AlbertEngine* AlbertEngine::_instance = nullptr;

/**********************************************************************//**
 * @brief AlbertEngine::AlbertEngine
 * @param parent
 */
AlbertEngine::AlbertEngine()
{
	// _websearch =
	// _calculator =
	_indizes.push_back(new FileSystemIndex);
}

/**********************************************************************//**
 * @brief AlbertEngine::~AlbertEngine
 */
AlbertEngine::~AlbertEngine()
{
}

/**********************************************************************//**
 * @brief AlbertEngine::instance
 * @return
 */
AlbertEngine *AlbertEngine::instance(){
	if (_instance == nullptr)
		_instance = new AlbertEngine;
	return _instance;
}

/**********************************************************************//**
 * @brief AlbertEngine::request
 * @param req
 * @return
 */
std::vector<AbstractServiceProvider::AbstractItem*> AlbertEngine::query(const std::string &req)
{
	_result.clear();
	for (auto i: _indizes) {
		const std::vector<AbstractServiceProvider::AbstractItem *> &r = i->query(req);
		_result.insert(_result.end(), r.begin(), r.end());
	}
	return _result;
}

/**********************************************************************//**
 * @brief AlbertEngine::buildIndex
 */
void AlbertEngine::buildIndex()
{
	for (auto i: _indizes) {
		i->buildIndex();
	}
}
