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
PluginLoader::PluginLoader(QString path) : QPluginLoader(path) {
    _status = Status::NotLoaded;
}



/** ***************************************************************************/
PluginLoader::~PluginLoader(){

}


/** ***************************************************************************/
QObject *PluginLoader::instance()
{
    load();
    return QPluginLoader::isLoaded() ? QPluginLoader::instance() : nullptr;
}

/** ***************************************************************************/
void PluginLoader::load(){
    if (QPluginLoader::load())
        _status = Status::Loaded;
    else {
        qWarning() << "Loading extension failed:" << fileName() << errorString();
        _status = Status::Error;
    }
}



/** ***************************************************************************/
void PluginLoader::unload(){
    if (QPluginLoader::unload())
        _status = Status::NotLoaded;
    else
        qWarning() << "Unloading extension failed:" << fileName() << errorString();
}



/** ***************************************************************************/
PluginLoader::Status PluginLoader::status() const {
    return _status;
}



/** ***************************************************************************/
QString PluginLoader::IID() const {
    return metaData()["IID"].toString();
}



/** ***************************************************************************/
QString PluginLoader::id() const {
    return metaData()["MetaData"].toObject()["id"].toString();
}



/** ***************************************************************************/
QString PluginLoader::name() const {
    return metaData()["MetaData"].toObject()["name"].toString();
}



/** ***************************************************************************/
QString PluginLoader::version() const {
    return metaData()["MetaData"].toObject()["version"].toString();
}



/** ***************************************************************************/
QString PluginLoader::platform() const {
    return metaData()["MetaData"].toObject()["platform"].toString();
}



/** ***************************************************************************/
QString PluginLoader::group() const {
    return metaData()["MetaData"].toObject()["group"].toString();
}



/** ***************************************************************************/
QString PluginLoader::copyright() const {
    return metaData()["MetaData"].toObject()["copyright"].toString();
}



/** ***************************************************************************/
QString PluginLoader::description() const {
    return metaData()["MetaData"].toObject()["description"].toString();
}



/** ***************************************************************************/
QStringList PluginLoader::dependencies()const{
    QStringList res;
    for (const QJsonValue &v : metaData()["MetaData"].toObject()["dependencies"].toArray())
        res << v.toString();
    return res;
}

