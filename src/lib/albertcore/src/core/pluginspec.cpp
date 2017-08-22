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
#include "plugin.h"



/** ***************************************************************************/
Core::PluginSpec::PluginSpec(const QString &path) : loader_(path) {
    iid_          = loader_.metaData()["IID"].toString();
    id_           = metadata("id").toString();
    name_         = metadata("name").toString("N/A");
    version_      = metadata("version").toString("N/A");
    author_       = metadata("author").toString("N/A");
    dependencies_ = metadata("dependencies").toVariant().toStringList();
    state_        = State::NotLoaded;
}


/** ***************************************************************************/
QString Core::PluginSpec::path() const {
    return loader_.fileName();
}


/** ***************************************************************************/
QString Core::PluginSpec::iid() const {
    return iid_;
}


/** ***************************************************************************/
QString Core::PluginSpec::id() const {
    return id_;
}


/** ***************************************************************************/
QString Core::PluginSpec::name() const {
    return name_;
}


/** ***************************************************************************/
QString Core::PluginSpec::version() const {
    return version_;
}


/** ***************************************************************************/
QString Core::PluginSpec::author() const {
    return author_;
}


/** ***************************************************************************/
QStringList Core::PluginSpec::dependencies() const {
    return dependencies_;
}


/** ***************************************************************************/
QJsonValue Core::PluginSpec::metadata(const QString &key) const {
    return loader_.metaData()["MetaData"].toObject()[key];
}


/** ***************************************************************************/
bool Core::PluginSpec::load() {

    Plugin *plugin = nullptr;
    if ( state_ != State::Loaded ) {
        try {
            if ( !loader_.instance() )
                lastError_ = loader_.errorString();
            else if ( ! (plugin = dynamic_cast<Plugin*>(loader_.instance())) )
                lastError_ = "Plugin instance is not of type Plugin";
            else {
                state_ = State::Loaded;
                plugin->id_ = this->id_;
            }
        } catch (const std::exception& ex) {
            lastError_ = ex.what();
        } catch (const std::string& s) {
            lastError_ = QString::fromStdString(s);
        } catch (const QString& s) {
            lastError_ = s;
        } catch (const char *s) {
            lastError_ = s;
        } catch (...) {
            lastError_ = "Unkown exception in plugin constructor.";
        }

        if (!plugin) {
           qWarning() << qPrintable(QString("Failed loading plugin: %1 [%2]").arg(path()).arg(lastError_));
           state_ = State::Error;
        }
    }

    return state_ == State::Loaded;
}


/** ***************************************************************************/
void Core::PluginSpec::unload(){

    /*
     * Never really unload a plugin, since otherwise all objects instanciated by
     * this extension (items, widgets, etc) and spread all over the app would
     * have to be deleted. This is a lot of work and nobody cares about that
     * little amount of extra KBs in RAM until next restart.
     */

    if ( state_ != State::NotLoaded ) {
        loader_.instance()->deleteLater();
        state_ = State::NotLoaded;
    }
}


/** ***************************************************************************/
Core::PluginSpec::State Core::PluginSpec::state() const {
    return state_;
}


/** ***************************************************************************/
QString Core::PluginSpec::lastError() const {
    return state_ == State::Error ? lastError_ : QString();
}


/** ***************************************************************************/
QObject *Core::PluginSpec::instance() {
    return (state_ == State::Loaded) ? loader_.instance() : nullptr;
}

