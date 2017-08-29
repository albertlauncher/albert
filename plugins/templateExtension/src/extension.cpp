// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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

#include <QDebug>
#include <QPointer>
#include <stdexcept>
#include "configwidget.h"
#include "core/item.h"
#include "extension.h"
#include "core/query.h"



class ProjectNamespace::Private
{
public:
    QPointer<ConfigWidget> widget;
};



/** ***************************************************************************/
ProjectNamespace::Extension::Extension()
    : Core::Extension("org.albert.extension.projectid"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    /*
     * Check the Extension and Plugin header to see the members in this scope
     */

    // You can throw in the constructor if something fatal happened
    throw std::runtime_error( "Description of error." );
    throw std::string( "Description of error." );
    throw QString( "Description of error." );
    throw "Description of error.";
    throw; // Whatever prints "unknown error"
}



/** ***************************************************************************/
ProjectNamespace::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *ProjectNamespace::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}



/** ***************************************************************************/
void ProjectNamespace::Extension::setupSession() {

}



/** ***************************************************************************/
void ProjectNamespace::Extension::teardownSession() {

}



/** ***************************************************************************/
void ProjectNamespace::Extension::handleQuery(Core::Query * query) {

    // Queries can be empty
    if ( query->searchTerm().isEmpty() )
        return;

    // If you registered a trigger and it matches trigger will contain it
    if ( !query->trigger().isNull() ) {

        /*
         * Use
         *
         *   query->addMatch(my_item)
         *
         * to add matches. If you created a throw away item MOVE it instead of
         * copying e.g.:
         *
         *   query->addMatch(std::move(my_tmp_item))
         *
         * The relevance factor is optional. (Defaults to 0) its a usigned integer depicting the
         * relevance of the item 0 mean not relevant UINT_MAX is totally relevant (exact match).
         * E.g. it the query is "it" and your items name is "item"
         *
         *   my_item.name().startswith(query->searchterm)
         *
         * is a naive match criterion and
         *
         *   UINT_MAX / ( query.searchterm().size() / my_item.name().size() )
         *
         * a naive match factor.
         *
         * If you have a lot of items use the iterator versions addMatches, e.g. like that
         *
         *   query->addMatches(my_items.begin(), my_items.end());
         *
         * If the items in the container are temporary object move them to avoid uneccesary
         * reference counting:
         *
         *   query->addMatches(std::make_move_iterator(my_tmp_items.begin()),
         *                     std::make_move_iterator(my_tmp_items.end()));
         */
    }
}

