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
#include "extensionloader.h"
#include "extension.h"
#include "nativeextensionloader.h"



/** ***************************************************************************/
bool Core::NativeExtensionLoader::load(){
    state_ = loader_.load() ? State::Loaded : State::Error;
    return state_==State::Loaded;
}



/** ***************************************************************************/
bool Core::NativeExtensionLoader::unload(){
    state_ = loader_.unload() ? State::NotLoaded : State::Error;
    return state_==State::NotLoaded;
}



/** ***************************************************************************/
QString Core::NativeExtensionLoader::lastError() const {
    return (state_==State::Error) ? loader_.errorString() : QString();
}



/** ***************************************************************************/
Core::Extension *Core::NativeExtensionLoader::instance() {
    return (state_==State::Loaded) ? dynamic_cast<Extension*>(loader_.instance()) : nullptr;
}



/** ***************************************************************************/
QString Core::NativeExtensionLoader::path() const {
    return loader_.fileName();
}



/** ***************************************************************************/
QString Core::NativeExtensionLoader::type() const {
    return "Native/C++";
}



/** ***************************************************************************/
QString Core::NativeExtensionLoader::id() const {
    return loader_.metaData()["MetaData"].toObject()["id"].toString();
}



/** ***************************************************************************/
QString Core::NativeExtensionLoader::name() const {
    return loader_.metaData()["MetaData"].toObject()["name"].toString();
}



/** ***************************************************************************/
QString Core::NativeExtensionLoader::version() const {
    return loader_.metaData()["MetaData"].toObject()["version"].toString();
}



/** ***************************************************************************/
QString Core::NativeExtensionLoader::author() const {
    return loader_.metaData()["MetaData"].toObject()["author"].toString();
}



/** ***************************************************************************/
QStringList Core::NativeExtensionLoader::dependencies() const {
    QStringList res;
    for (QVariant &var : loader_.metaData()["MetaData"].toObject()["dependencies"].toArray().toVariantList())
        res.push_back(var.toString());
    return res;
}
