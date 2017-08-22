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
    : Core::Extension("org.albert.extension.projectid"),
      Core::QueryHandler(Core::Extension::id),
      d(new Private) {

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

    if ( query->isTriggered() ) {

    } else {

    }
}

