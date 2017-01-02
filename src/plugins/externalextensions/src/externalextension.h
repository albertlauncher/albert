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


// Communication between Albert and external extensions
// =====================================================================
//
// The need for this protocol is the desire to externalize extensions into
// executables. This will make developing extension less complicated, faster,
// and accesible to a broader community (i.e. other than C++ programmers).
// However this extensions are separate executables which has several
// implications a developer should be aware of.
//
// * Executable extensions are run and are intended to terminate. This means all
//   state has to be de-/serialized from/to disk. Data intensive extension
//   should therefore implemented natively.
// * An extension is a separate process and therefore has no access to the memory
//   of the core application. This means there is no access to core components,
//   e.g. the app interface itself, icon provider libraries, etc.
// * Although strictly a follow up of the latter it has to be stated that the
//   communication is synchronous in time and fixed by the communication
//   protocol. Its also worth to mention that, due to the synchronicity of the
//   protcol it is not possible to return asynchronous "live" results.
//
// The core application triggers the extension by running them with an OPCODE.
// The OPCODE is always the first positional argument and may be followed by
// further arguments the operation defined by OPCODE requires. If the operation
// needs to send back information it can pass print it to stdout. The core
// application will then try to parse it according to the specifications defined
// in the communications protocol.
//
// Specification of communication protocol org.albert.extension.external.v1
// ========================================================================
//
// v1 comprises the following opcodes
//
// * METADATA
//   The application wants to get the metadata of the extenion. The metadata
//   shall be returned in the form of a JSON object. It should have the
//   following keys:
//
//    * iid (string, mandatory)
//    * id (string, mandatory)
//    * version (string, defaults to 'N/A')
//    * name (string, defaults to $id)
//    * trigger (string, defaults to 'empty')
//    * author (string, defaults to 'N/A')
//    * dependencies (array of strings, defaults to 'empty')
//
//   The interface id 'iid' tells the application the type and version of the
//   communication protocol. If the iid is incompatible this extension will not
//   show up in the extensions list.
//   The id is an identifier. Each id will be loaded only once.
//   The remaining keys should be self explanatory.
//
// * INITIALIZE
//   A signal sent when the user loaded the extension.
//   Further the demand to check if the requirements for this extension are met.
//   Return an error message in case of errors. If the output is empty everything
//   is assumed to be fine. If this is not the case the extension will not be
//   enabled.
//
// * FINALIZE
//   A signal sent when the user unloaded the extension.
//
// * SETUPSESSION
//   A signal sent when the user started a session.
//
// * TEARDOWNSESSION
//   A signal sent when the user ended a session.
//
// * QUERY
//   The request to handle a query. The first argument after the OPCODE is the
//   query term. Return the results by a JSON array containing JSON objects
//   represeting the results. A result object has four values: its extension
//   wide unique id ('id':string), its name ('name':string), its description
//   ('description':string), its icon ('icon':string), which depends on the
//   platfrom to be a name or path, and a JSON array of JSON objects
//   ('actions':array) containing the actions for the item. An action object
//   contains the actions name ('name':string), the program to be runned
//   ('command':string) and an array of parameters ('arguments':array) that
//   should be passed to the program. An example:
//
//    [{
//      "id":"extension.wide.unique.id",
//      "name":"An Item",
//      "description":"Nice description.",
//      "icon":"/path/to/icon",
//      "actions":[{
//        "name":"Action name",
//        "command":"program",
//        "arguments":["-a", "-b"]
//      },{
//        ...
//      }]
//    },{
//     ...
//    }]

#pragma once
#include <QObject>
#include "extension.h"
#include "queryhandler.h"

namespace ExternalExtensions {

class ExternalExtension final : public QObject, public Core::QueryHandler
{

public:

    ExternalExtension(const QString &path,
                      const QString &id,
                      const QString &name,
                      const QString &author,
                      const QString &version,
                      const QString &trigger,
                      const QStringList &dependencies);
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

    const QString path;
    const QString id;
    const QString name;
    const QString author;
    const QString version;
    const QStringList dependencies;

private:

    const QString trigger_;
};

}
