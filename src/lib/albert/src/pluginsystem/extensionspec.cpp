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
#include <QDebug>
#include <stdexcept>
#include "extensionspec.h"
#include "extension.h"



/** ***************************************************************************/
bool Core::ExtensionSpec::load(){
    if ( loader_.load() ) {
        state_ = State::Loaded;
    } else {
        state_ = State::Error;
        lastError_ = loader_.errorString();
        qWarning() << "Failed to load extension:" << lastError_.toLocal8Bit().data();
    }
    return state_==State::Loaded;
}



/** ***************************************************************************/
bool Core::ExtensionSpec::unload(){
    if ( loader_.unload() ) {
        state_ = State::NotLoaded;
    } else {
        state_ = State::Error;
        lastError_ = loader_.errorString();
        qWarning() << "Failed to unload extension:" << lastError_.toLocal8Bit().data();
    }
    return state_==State::NotLoaded;
}



/** ***************************************************************************/
QString Core::ExtensionSpec::lastError() const {
    return (state_==State::Error) ? lastError_ : QString();
}



/** ***************************************************************************/
QObject *Core::ExtensionSpec::instance() {

    auto errorHandler = [this]() {
        state_ = State::Error;
        qWarning() << id() << "Failed to instanciate extension:" << lastError_.toLocal8Bit().data();
        return nullptr;
    };

    try {
        return (state_==State::Loaded) ? loader_.instance() : nullptr;
    } catch (const std::exception& ex) {
        lastError_ = ex.what();
        return errorHandler();
    } catch (const std::string& s) {
        lastError_ = QString::fromStdString(s);
        return errorHandler();
    } catch (const QString& s) {
        lastError_ = s;
        return errorHandler();
    } catch (const char *s) {
        lastError_ = s;
        return errorHandler();
    } catch (...) {
        lastError_ = "Unkown exception in extension constructor.";
        return errorHandler();
    }
}



/** ***************************************************************************/
QString Core::ExtensionSpec::path() const {
    return loader_.fileName();
}



/** ***************************************************************************/
QString Core::ExtensionSpec::id() const {
    return loader_.metaData()["MetaData"].toObject()["id"].toString();
}



/** ***************************************************************************/
QString Core::ExtensionSpec::name() const {
    return loader_.metaData()["MetaData"].toObject()["name"].toString();
}



/** ***************************************************************************/
QString Core::ExtensionSpec::version() const {
    return loader_.metaData()["MetaData"].toObject()["version"].toString();
}



/** ***************************************************************************/
QString Core::ExtensionSpec::author() const {
    return loader_.metaData()["MetaData"].toObject()["author"].toString();
}



/** ***************************************************************************/
QStringList Core::ExtensionSpec::dependencies() const {
    QStringList res;
    for (QVariant &var : loader_.metaData()["MetaData"].toObject()["dependencies"].toArray().toVariantList())
        res.push_back(var.toString());
    return res;
}
