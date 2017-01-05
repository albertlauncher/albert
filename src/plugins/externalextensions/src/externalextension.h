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


/*
 * Communication between Albert and external extensions
 * =====================================================================
 *
 * The need for this protocol is the desire to externalize extensions into
 * executables. This will make developing extension less complicated, faster,
 * and accesible to a broader community (i.e. other than C++ programmers).
 *
 * There are three unidirectional communication channels. The environment
 * variables, the standard output (stdout) and the exit codes of the program.
 *
 * The workflow is as follows: Albert runs the external extension with several
 * environment variables set. The environment variable ALBERT_OP is always set.
 * ALBERT_OP can contain several commands, which together form the interface of
 * an external extension. The set of commands is specified below.
 *
 * With the exit code external extensions can signal errors to the main
 * application. Depending on the ALBERT_OP reactions may differ, but at least
 * a warning is printed.
 *
 * Stdout is used to get responses. Depending on the ALBERT_OP the expected
 * data may differ, but all responses have to be a JSON object containg data.
 *
 * To save state between the executions the extension can return a json object
 * called "variables". The properties of the object "variables" will be set as
 * environment variables in the next execution. Note that this properties have
 * to be strings otherwise they will not be set in the environment.
 *
 * The applications ensures that the extension is not executed multiple times
 * concurrently.
 *
 * Specification of communication protocol org.albert.extension.external.v2
 * ========================================================================
 *
 * v1 comprises the following ALBERT_OPs
 *
 * * METADATA
 *
 * The application wants to get the metadata of the extenion. It should have the
 * following keys:
 *
 *  * iid (string, mandatory)
 *  * version (string, defaults to 'N/A')
 *  * name (string, defaults to $id)
 *  * trigger (string, defaults to 'empty')
 *  * author (string, defaults to 'N/A')
 *  * dependencies (array of strings, defaults to 'empty')
 *
 * The interface id 'iid' tells the application the type and version of the
 * communication protocol. If the iid is incompatible this extension will not
 * show up in the extensions list. The remaining keys should be self
 * explanatory.
 *
 * Errors in this step are fatal: loading will not be continued.
 *
 * * INITIALIZE
 *
 * The request to initialize the extension. The extension should check if all
 * requirements are met and set the exit code accordingly. (Everything but  zero
 *  is an error).
 *
 * Errors in this step are fatal: loading will not be continued.
 *
 *
 * * FINALIZE
 *
 * The request to finalize the extension.
 *
 * * SETUPSESSION
 *
 * The request to setup for a session, meaning prepare for user queries.
 *
 * * TEARDOWNSESSION
 *
 * The request to teardown a session.
 *
 * * QUERY
 *
 * The request to handle a query. The environment variable ALBERT_QUERY contains
 * the complete query as the user enterd it into the input box, i.e. including
 * potential triggers.
 *
 * Return the results by a JSON array containing JSON objects represeting the
 * results. A result object has four values: its extension wide unique id
 * ('id':string), its name ('name':string), its description ('description':
 * string), its icon ('icon':string), which depends on the platfrom to be a name
 * or path, and a JSON array of JSON objects ('actions':array) containing the
 * actions for the item. An action object contains the actions name ('name':
 * string), the program to be runned ('command':string) and an array of
 * parameters ('arguments':array) that should be passed to the program.
 *
 * An example:
 *
 * {
 *   "items": [{
 *     "id":"extension.wide.unique.id",
 *     "name":"An Item",
 *     "description":"Nice description.",
 *     "icon":"/path/to/icon",
 *     "actions":[{
 *       "name":"Action name 1",
 *       "command":"program",
 *       "arguments":["-a", "-b"]
 *     },{
 *       "name":"Action name 2",
 *       "command":"program2",
 *       "arguments":["-C", "-D"]
 *     }]
 *   }],
 *   "variables": {
 *     "env_var":"some variable",
 *     "state":"something representing state"
 *   }
 * }
 *
 */

#pragma once
#include <QObject>
#include <QProcess>
#include <QMutex>
#include <map>
#include "queryhandler.h"

namespace ExternalExtensions {

class ExternalExtension final : public QObject, public Core::QueryHandler
{

public:

    ExternalExtension(const QString &path, const QString &id);
    ~ExternalExtension();

    /*
     * Implementation of extension interface
     */

    QString trigger() const override { return trigger_; }
    void setupSession() override;
    void teardownSession() override;
    void handleQuery(Core::Query *query) override;

    /*
     * Extension specific members
     */


    const QString &path() { return path_; }
    const QString &id() { return id_; }
    const QString &name() { return name_; }
    const QString &author() { return author_; }
    const QString &version() { return version_; }
    const QStringList &dependencies() { return dependencies_; }

private:

    QString runOperation(const QString &);

    QString path_;
    QString id_;
    QString name_;
    QString author_;
    QString version_;
    QStringList dependencies_;
    QString trigger_;
    std::map<QString, QString> variables_;
    QMutex processMutex_;
};

}
