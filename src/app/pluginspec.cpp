// Copyright (C) 2014-2018 Manuel Schneider

#include <QDebug>
#include <QJsonArray>
#include <QVariant>
#include <QSettings>
#include <stdexcept>
#include "pluginspec.h"
#include "albert/plugin.h"


/** ***************************************************************************/
Core::PluginSpec::PluginSpec(const QString &path)
    : loader_(path) {

    // Some python libs do not link against python. Export the python symbols to the main app.
    loader_.setLoadHints(QLibrary::ExportExternalSymbolsHint);

    iid_          = loader_.metaData()["IID"].toString();
    id_           = metadata("id").toString();
    name_         = metadata("name").toString("N/A");
    version_      = metadata("version").toString("N/A");
    author_       = metadata("author").toString("N/A");
    dependencies_ = metadata("dependencies").toVariant().toStringList();
    state_        = State::NotLoaded;
}


/** ***************************************************************************/
Core::PluginSpec::~PluginSpec() {
    if ( state_ != State::NotLoaded )
        unload();
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
            else
                state_ = State::Loaded;
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
           loader_.unload();
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

    if ( state_ == State::Loaded )
        delete loader_.instance();
    state_ = State::NotLoaded;
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

