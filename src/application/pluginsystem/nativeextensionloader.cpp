// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#include <QJsonArray>
#include <QPluginLoader>
#include <QVariant>
#include "abstractextensionloader.h"
#include "abstractextension.h"
#include "nativeextensionloader.h"



/** ***************************************************************************/
bool NativeExtensionLoader::load(){
    QPluginLoader loader(path_);
    if (loader.load()){
        state_ = State::Loaded;
        return true;
    }
    lastError_ = QString("%1: %2").arg(path_, loader.errorString());
    state_ = State::Error;
    return false;
}



/** ***************************************************************************/
bool NativeExtensionLoader::unload(){
    QPluginLoader loader(path_);
    if (loader.unload()){
        state_ = State::NotLoaded;
        return true;
    }
    lastError_ = QString("%1: %2").arg(path_, loader.errorString());
    state_ = State::Error;
    return false;
}



/** ***************************************************************************/
QString NativeExtensionLoader::lastError() const {
    return lastError_;
}



/** ***************************************************************************/
AbstractExtension *NativeExtensionLoader::instance() {
    if (state_ == State::Loaded)
        return qobject_cast<AbstractExtension*>(QPluginLoader(path_).instance());
    else
        return nullptr;
}



/** ***************************************************************************/
QString NativeExtensionLoader::path() const {
    return path_;
}



/** ***************************************************************************/
QString NativeExtensionLoader::type() const {
    return "Native/C++";
}



/** ***************************************************************************/
QString NativeExtensionLoader::id() const {
    return QPluginLoader(path_).metaData()["MetaData"].toObject()["id"].toString();
}



/** ***************************************************************************/
QString NativeExtensionLoader::name() const {
    return QPluginLoader(path_).metaData()["MetaData"].toObject()["name"].toString();
}



/** ***************************************************************************/
QString NativeExtensionLoader::version() const {
    return QPluginLoader(path_).metaData()["MetaData"].toObject()["version"].toString();
}



/** ***************************************************************************/
QString NativeExtensionLoader::author() const {
    return QPluginLoader(path_).metaData()["MetaData"].toObject()["author"].toString();
}



/** ***************************************************************************/
QStringList NativeExtensionLoader::dependencies() const {
    QStringList res;
    for (QVariant &var : QPluginLoader(path_).metaData()["MetaData"].toObject()["dependencies"].toArray().toVariantList())
        res.push_back(var.toString());
    return res;
}
