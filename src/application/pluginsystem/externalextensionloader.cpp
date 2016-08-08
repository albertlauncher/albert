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


#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPluginLoader>
#include <QProcess>
#include <QVariant>
#include "externalextensionloader.h"
#include "externalextension.h"

#define EXTERNAL_EXTENSION_IID "org.albert.extension.external.v1"

/** ***************************************************************************/
ExternalExtensionLoader::ExternalExtensionLoader(QString path) :
    path_(path), instance_(nullptr){

    // Get Metadata
    QProcess extProc;
    extProc.start(path, {"METADATA"});
    if (!extProc.waitForFinished(1000))
        throw QString("Communication timed out.");

    // Read JSON data
    QJsonDocument doc = QJsonDocument::fromJson(extProc.readAllStandardOutput());
    if (doc.isNull() || !doc.isObject())
        throw QString("Reply to 'METADATA' is not a valid JSON object.");

    QJsonObject metadata = doc.object();

    // Check for a sane interface ID (IID)
    QString iid = metadata["iid"].toString();
    if (iid != EXTERNAL_EXTENSION_IID)
        throw QString("Interface id '' does not match ''.").arg(iid, EXTERNAL_EXTENSION_IID);

    // Check for mandatory id
    if (metadata["id"].isUndefined())
        throw QString("Metadata does not contain an extension id.");
    id_ = metadata["id"].toString();
    if (id_.isEmpty())
        throw QString("Extension id is empty.");

    // Get opional data
    name_ = metadata["name"].toString("N/A");
    version_ = metadata["version"].toString("N/A");
    author_ = metadata["author"].toString("N/A");
    for (const QJsonValue & value : metadata["dependencies"].toArray())
         dependencies_.append(value.toString());
}



/** ***************************************************************************/
ExternalExtensionLoader::~ExternalExtensionLoader() {
    if (instance_ != nullptr)
        unload();
}



/** ***************************************************************************/
bool ExternalExtensionLoader::load(){
    if (instance_ == nullptr) {
        try {
            instance_ = new ExternalExtension(qUtf8Printable(id()), path_);
            state_ = State::Loaded;
        } catch (QString error) {
            state_ = State::Error;
            lastError_ = error;
            qWarning() << lastError_;
            return false;
        }
    }
    return true;
}



/** ***************************************************************************/
bool ExternalExtensionLoader::unload(){
    if (instance_ != nullptr) {
        delete instance_;
        instance_ = nullptr;
        state_ = State::NotLoaded;
    }
    return true;
}



/** ***************************************************************************/
QString ExternalExtensionLoader::lastError() const {
    return lastError_;
}



/** ***************************************************************************/
AbstractExtension *ExternalExtensionLoader::instance() {
    return instance_;
}



/** ***************************************************************************/
QString ExternalExtensionLoader::path() const {
    return path_;
}



/** ***************************************************************************/
QString ExternalExtensionLoader::type() const {
    return "External/Executable";
}
