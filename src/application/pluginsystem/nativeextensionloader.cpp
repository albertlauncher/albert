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
#include <QVariant>
#include "abstractextensionloader.h"
#include "abstractextension.h"
#include "nativeextensionloader.h"



/** ***************************************************************************/
bool NativeExtensionLoader::load(){
    state_ = loader_.load() ? State::Loaded : State::Error;
    return state_==State::Loaded;
}



/** ***************************************************************************/
bool NativeExtensionLoader::unload(){
    state_ = loader_.unload() ? State::NotLoaded : State::Error;
    return state_==State::NotLoaded;
}



/** ***************************************************************************/
QString NativeExtensionLoader::lastError() const {
    return (state_==State::Error) ? loader_.errorString() : QString();
}



/** ***************************************************************************/
AbstractExtension *NativeExtensionLoader::instance() {
    return (state_==State::Loaded) ? qobject_cast<AbstractExtension*>(loader_.instance()) : nullptr;
}



/** ***************************************************************************/
QString NativeExtensionLoader::path() const {
    return loader_.fileName();
}



/** ***************************************************************************/
QString NativeExtensionLoader::type() const {
    return "Native/C++";
}



/** ***************************************************************************/
QString NativeExtensionLoader::id() const {
    return loader_.metaData()["MetaData"].toObject()["id"].toString();
}



/** ***************************************************************************/
QString NativeExtensionLoader::name() const {
    return loader_.metaData()["MetaData"].toObject()["name"].toString();
}



/** ***************************************************************************/
QString NativeExtensionLoader::version() const {
    return loader_.metaData()["MetaData"].toObject()["version"].toString();
}



/** ***************************************************************************/
QString NativeExtensionLoader::author() const {
    return loader_.metaData()["MetaData"].toObject()["author"].toString();
}



/** ***************************************************************************/
QStringList NativeExtensionLoader::dependencies() const {
    QStringList res;
    for (QVariant &var : loader_.metaData()["MetaData"].toObject()["dependencies"].toArray().toVariantList())
        res.push_back(var.toString());
    return res;
}
