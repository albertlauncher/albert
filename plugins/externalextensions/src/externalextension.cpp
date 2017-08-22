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

#include "externalextension.h"
#include <QByteArray>
#include <QFileInfo>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QProcess>
#include <QVBoxLayout>
#include <functional>
#include <vector>
#include "core/query.h"
#include "util/standardaction.h"
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
using std::pair;
using std::shared_ptr;
using std::vector;
using namespace Core;

#define EXTERNAL_EXTENSION_IID "org.albert.extension.external/v2.0"

namespace {

bool runProcess (QString path,
                 std::map<QString, QString> *variables,
                 QByteArray *out,
                 QString *errorString) {

    // Run the process
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for ( auto & entry : *variables )
        env.insert(entry.first, entry.second);
    process.setProcessEnvironment(env);
    process.setProgram(path);
    process.start();
    process.waitForFinished(-1);

    if ( process.exitStatus() != QProcess::NormalExit ) {
        *errorString = QString("Process crashed.");
        return false;
    }

    if ( process.exitCode() != 0 ) {

        *errorString = QString("Exit code is %1.").arg(process.exitCode());

        QByteArray cout = process.readAllStandardOutput();
        QByteArray cerr = process.readAllStandardError();

        if (!cout.isEmpty())
            errorString->append(QString("\n%1").arg(QString(cout)));

        if (!cerr.isEmpty())
            errorString->append(QString("\n%1").arg(QString(cerr)));

        return false;
    }

    *out = process.readAllStandardOutput();

    return true;
}


bool parseJsonObject (const QByteArray &json,
                      QJsonObject *object,
                      QString *errorString) {

    // Parse stdout
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(json, &error);
    if ( document.isNull() ) {
        *errorString = QString("Invalid JSON at %1: %2").arg(error.offset).arg(error.errorString());
        return false;
    }

    *object = document.object();
    if ( object->isEmpty() ) {
        *errorString = QString("Expected json object, but received an array.");
        return false;
    }

    return true;
}


bool saveVariables (QJsonObject *object,
                    std::map<QString, QString> *variables,
                    QString *errorString) {

    variables->clear();

    if ( !object->contains("variables") )
        return true;

    if ( !object->operator[]("variables").isObject() ) {
        *errorString = "'variables' is not a JSON object";
        return false;
    }

    QJsonObject vars = object->operator[]("variables").toObject();
    for (auto it = vars.begin(); it != vars.end(); ++it)
        if ( it.value().isString() )
            variables->emplace(it.key(), it.value().toString());

    return true;
}

}

/** ***************************************************************************/
ExternalExtensions::ExternalExtension::ExternalExtension(const QString &path, const QString &id)
    : QueryHandler(id), path_(path) {

    // Never run the extension concurrent
    QMutexLocker lock (&processMutex_);


    /*
     * Get the metadata
     */

    // Run the process
    variables_["ALBERT_OP"] = "METADATA";
    QString errorString;
    QByteArray out;
    if ( !runProcess(path_, &variables_, &out, &errorString) )
        throw QString("Getting metadata failed: %1 (%2)").arg(errorString, path_);

    // Parse stdout
    QJsonObject object;
    if ( !parseJsonObject(out, &object,  &errorString) )
        throw QString("Getting metadata failed: %1 (%2)").arg(errorString, path_);

    // Check for a sane interface ID (IID)
    if (object["iid"].isUndefined())
        throw QString("Getting metadata failed: Does not contain an interface id. (%1)").arg(path_);

    QString iid = object["iid"].toString();
    if (iid != EXTERNAL_EXTENSION_IID)
        throw QString("Getting metadata failed: Interface id '%1' does not match '%2'. (%3)").arg(iid, EXTERNAL_EXTENSION_IID, path_);

    // Get opional data
    QJsonValue val;

    val = object["trigger"];
    trigger_ = val.isString() ? val.toString() : QString();

    val = object["name"];
    name_ = val.isString() ? val.toString() : id;

    val = object["version"];
    version_ = val.isString() ? val.toString() : "N/A";

    val = object["author"];
    author_ = val.isString() ? val.toString() : "N/A";

    QStringList dependencies;
    for (const QJsonValue & value : object["dependencies"].toArray())
         dependencies.append(value.toString());


    /*
     * Initialize the extension
     */

    // Run the process
    variables_["ALBERT_OP"] = "INITIALIZE";
    if ( !runProcess(path, &variables_, &out,  &errorString) )
        throw QString("Initialization failed: %1 (%2)").arg(errorString, path_);

    if ( out.isEmpty() )
        return;

    // Parse stdout
    if ( !parseJsonObject(out, &object,  &errorString) )
        throw QString("Initialization failed: %1 (%2)").arg(errorString, path_);

    // Finally save the variables, if any
    if ( !saveVariables(&object, &variables_, &errorString) )
        qWarning() << qPrintable(QString("Initialization: %1 (%2)").arg(errorString, path_));
}


/** ***************************************************************************/
ExternalExtensions::ExternalExtension::~ExternalExtension() {

    // Never run the extension concurrent
    QMutexLocker lock (&processMutex_);
    QString errorString;
    QJsonObject object;
    QByteArray out;

    // Run the process
    variables_["ALBERT_OP"] = "FINALIZE";
    if ( !runProcess(path_, &variables_, &out, &errorString) ) {
        qWarning() << qPrintable(QString("Finalization failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    if ( out.isEmpty() )
        return;

    // Parse stdout
    if ( !parseJsonObject(out, &object,  &errorString) ) {
        qWarning() << qPrintable(QString("Finalization failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    // Save the variables, if any
    if ( !saveVariables(&object, &variables_, &errorString) ){
        qWarning() << qPrintable(QString("Finalization: %1 (%2)").arg(errorString, path_));
        return;
    }
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtension::setupSession() {

    // Never run the extension concurrent
    QMutexLocker lock (&processMutex_);
    QString errorString;
    QJsonObject object;
    QByteArray out;

    // Run the process
    variables_["ALBERT_OP"] = "SETUPSESSION";
    if ( !runProcess(path_, &variables_, &out, &errorString) ) {
        qWarning() << qPrintable(QString("Session setup failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    if ( out.isEmpty() )
        return;

    // Parse stdout
    if ( !parseJsonObject(out, &object,  &errorString) ) {
        qWarning() << qPrintable(QString("Session setup failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    // Save the variables, if any
    if ( !saveVariables(&object, &variables_, &errorString) ){
        qWarning() << qPrintable(QString("Session setup: %1 (%2)").arg(errorString, path_));
        return;
    }
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtension::teardownSession() {

    // Never run the extension concurrent
    QMutexLocker lock (&processMutex_);
    QString errorString;
    QJsonObject object;
    QByteArray out;

    // Run the process
    variables_["ALBERT_OP"] = "TEARDOWNSESSION";
    if ( !runProcess(path_, &variables_, &out, &errorString) ) {
        qWarning() << qPrintable(QString("Session teardown failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    if ( out.isEmpty() )
        return;

    // Parse stdout
    if ( !parseJsonObject(out, &object,  &errorString) ) {
        qWarning() << qPrintable(QString("Session teardown failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    // Save the variables, if any
    if ( !saveVariables(&object, &variables_, &errorString) ){
        qWarning() << qPrintable(QString("Session teardown: %1 (%2)").arg(errorString, path_));
        return;
    }
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtension::handleQuery(Query* query) {

    // External extension must run only when triggered, since they are to ressource heavy
    if ( !query->isTriggered() )
        return;

    // Never run the extension concurrent
    QMutexLocker lock (&processMutex_);
    QString errorString;
    QJsonObject object;
    QByteArray out;

    // Run the process
    variables_["ALBERT_OP"] = "QUERY";
    variables_["ALBERT_QUERY"] = query->searchTerm();
    if ( !runProcess(path_, &variables_, &out, &errorString) ) {
        qWarning() << qPrintable(QString("Handle query failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    // Parse stdout
    if ( !parseJsonObject(out, &object,  &errorString) ) {
        qWarning() << qPrintable(QString("Handle query failed: %1 (%2)").arg(errorString, path_));
        return;
    }

    if ( out.isEmpty() )
        return;

    // Save the variables, if any
    if ( !saveVariables(&object, &variables_, &errorString) ){
        qWarning() << qPrintable(QString("Handle query: %1 (%2)").arg(errorString, path_));
        return;
    }

    // Check existance of items
    if ( !object.contains("items") ) {
        qWarning() << qPrintable(QString("Handle query failed: Result contains no items (%1)").arg(path_));
        return;
    }

    // Check type of items
    if ( !object["items"].isArray() ) {
        qWarning() << qPrintable(QString("Handle query failed: 'items' is not an array (%1)").arg(path_));
        return;
    }

    // Iterate over the results
    shared_ptr<StandardItem> standardItem;
    shared_ptr<StandardAction> standardAction;
    vector<shared_ptr<Action>> standardActionVector;
    vector<pair<shared_ptr<Core::Item>,short>> results;

    for (const QJsonValue & itemValue : object["items"].toArray() ){

        if ( !itemValue.isObject() ) {
            qWarning() << qPrintable(QString("Item is not a json object. (%1)").arg(path_));
            continue;
        }
        object = itemValue.toObject();

        // Build the item from the json object
        standardItem = std::make_shared<StandardItem>(object["id"].toString());
        standardItem->setText(object["name"].toString());
        standardItem->setSubtext(object["description"].toString());
        standardItem->setCompletionString(object["completion"].toString());
        QString icon = XDG::IconLookup::iconPath({object["icon"].toString(), "unknown"});
        standardItem->setIconPath(icon.isEmpty() ? ":unknown" : icon);

        // Build the actions
        standardActionVector.clear();
        for (const QJsonValue & value : object["actions"].toArray()){
            object = value.toObject();
            standardAction = std::make_shared<StandardAction>();
            standardAction->setText(object["name"].toString());
            QString command = object["command"].toString();
            QStringList arguments;
            for (const QJsonValue & value : object["arguments"].toArray())
                 arguments.append(value.toString());
            standardAction->setAction(std::bind(static_cast<bool(*)(const QString&,const QStringList&)>(&QProcess::startDetached), command, arguments));
            standardActionVector.push_back(standardAction);
        }
        standardItem->setActions(std::move(standardActionVector));

        results.emplace_back(std::move(standardItem), 0);
    }

    query->addMatches(results.begin(), results.end());
}

