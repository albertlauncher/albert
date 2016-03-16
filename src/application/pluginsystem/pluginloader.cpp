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
#include "pluginloader.h"

/** ***************************************************************************/
PluginSpec::PluginSpec(QString path) : QPluginLoader(path) {
    status_ = Status::NotLoaded;
}



/** ***************************************************************************/
PluginSpec::~PluginSpec() {

}


/** ***************************************************************************/
QObject *PluginSpec::instance() {
    load();
    return QPluginLoader::isLoaded() ? QPluginLoader::instance() : nullptr;
}

/** ***************************************************************************/
void PluginSpec::load() {
    if (QPluginLoader::load())
        status_ = Status::Loaded;
    else {
        qWarning() << "Loading extension failed:" << fileName() << errorString();
        status_ = Status::Error;
    }
}



/** ***************************************************************************/
void PluginSpec::unload() {
    if (QPluginLoader::unload())
        status_ = Status::NotLoaded;
    else
        qWarning() << "Unloading extension failed:" << fileName() << errorString();
}



/** ***************************************************************************/
PluginSpec::Status PluginSpec::status() const {
    return status_;
}



/** ***************************************************************************/
QString PluginSpec::IID() const {
    return metaData()["IID"].toString();
}



/** ***************************************************************************/
QString PluginSpec::id() const {
    return metaData()["MetaData"].toObject()["id"].toString();
}



/** ***************************************************************************/
QString PluginSpec::name() const {
    return metaData()["MetaData"].toObject()["name"].toString();
}



/** ***************************************************************************/
QString PluginSpec::version() const {
    return metaData()["MetaData"].toObject()["version"].toString();
}



/** ***************************************************************************/
QString PluginSpec::platform() const {
    return metaData()["MetaData"].toObject()["platform"].toString();
}



/** ***************************************************************************/
QString PluginSpec::group() const {
    return metaData()["MetaData"].toObject()["group"].toString();
}



/** ***************************************************************************/
QString PluginSpec::author() const {
    return metaData()["MetaData"].toObject()["author"].toString();
}



/** ***************************************************************************/
QString PluginSpec::description() const {
    return metaData()["MetaData"].toObject()["description"].toString();
}



/** ***************************************************************************/
QString PluginSpec::dependencies() const {
    return metaData()["MetaData"].toObject()["dependencies"].toString();
}

