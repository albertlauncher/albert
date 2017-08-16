// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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
#include <QJsonArray>
#include <QVariant>
#include <QSettings>
#include <stdexcept>
#include "pluginspec.h"



/** ***************************************************************************/
Core::PluginSpec::PluginSpec(const QString &path)
    : loader_(path) {

    state_ = loader_.isLoaded() ? State::Loaded : State::NotLoaded;
}



/** ***************************************************************************/
bool Core::PluginSpec::load() {

    if ( state_ == State::Loaded )
        return true;

    qDebug() << "Loading plugin" << path();

    if ( loader_.load() ) {
        state_ = State::Loaded;
        emit pluginLoaded(this);
    } else {
        state_ = State::Error;
        lastError_ = loader_.errorString();
        qWarning() << qPrintable(QString("%1 failed loading. %2").arg(id()).arg(loader_.errorString()));
    }
    return state_ == State::Loaded;
}



/** ***************************************************************************/
bool Core::PluginSpec::unload(){

    /*
     * Never really unload a plugin, since otherwise all objects instanciated by
     * this extension (items, widgets, etc) and spread all over the app would
     * have to be deleted. This is a lot of work and nobody cares about that
     * little amount of extra KBs in RAM until next restart.
     */

//    if ( loader_.unload() ) {
//        state_ = State::NotLoaded;
//    } else {
//        state_ = State::Error;
//        lastError_ = loader_.errorString();
//        qWarning() << "Failed to unload extension:" << lastError_.toLocal8Bit().data();
//    }
//    return state_==State::NotLoaded;

    if ( state_ == State::NotLoaded )
        return true;

    emit pluginAboutToUnload(this);
    delete instance();
    state_ = State::NotLoaded;
    return true;
}



/** ***************************************************************************/
QString Core::PluginSpec::lastError() const {
    return state_ == State::Error ? loader_.errorString() : QString();
}



/** ***************************************************************************/
QString Core::PluginSpec::iid() const {
    return loader_.metaData()["IID"].toString();
}



/** ***************************************************************************/
QJsonValue Core::PluginSpec::metadata(const QString &key) const {
    return loader_.metaData()["MetaData"].toObject()[key];
}



/** ***************************************************************************/
QObject *Core::PluginSpec::instance() {

    if ( state_ != State::Loaded )
        if ( !load() )
            return nullptr;

    auto errorHandler = [this]() {
        state_ = State::Error;
        qWarning() << loader_.fileName()
                   << "Failed to instanciate root component:"
                   << lastError_.toLocal8Bit().data();
    };

    try {
        return loader_.instance();
//    } catch (const std::exception& ex) {
//        lastError_ = ex.what();
//        errorHandler();
//    } catch (const std::string& s) {
//        lastError_ = QString::fromStdString(s);
//        errorHandler();
//    } catch (const QString& s) {
//        lastError_ = s;
//        errorHandler();
//    } catch (const char *s) {
//        lastError_ = s;
//        errorHandler();
    } catch (...) {
        lastError_ = "Unkown exception in plugin constructor.";
        errorHandler();
    }
    return nullptr;
}



/** ***************************************************************************/
QString Core::PluginSpec::path() const {
    return loader_.fileName();
}



/** ***************************************************************************/
QString Core::PluginSpec::id() const {
    return loader_.metaData()["MetaData"].toObject()["id"].toString();
}


/** ***************************************************************************/
QString Core::PluginSpec::name() const {
    return loader_.metaData()["MetaData"].toObject()["name"].toString();
}



/** ***************************************************************************/
QString Core::PluginSpec::version() const {
    return loader_.metaData()["MetaData"].toObject()["version"].toString();
}



/** ***************************************************************************/
QString Core::PluginSpec::author() const {
    return loader_.metaData()["MetaData"].toObject()["author"].toString();
}



/** ***************************************************************************/
QStringList Core::PluginSpec::dependencies() const {
    QStringList dependencies_;
    for (QVariant &var : loader_.metaData()["MetaData"].toObject()["dependencies"].toArray().toVariantList())
        dependencies_.push_back(var.toString());
    return dependencies_;
}

