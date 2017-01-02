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

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QProcess>
#include <QVBoxLayout>
#include <vector>
#include "query.h"
#include "externalextension.h"
#include "standarditem.h"
#include "standardaction.h"
#include "xdgiconlookup.h"
using std::vector;
using namespace Core;


/** ***************************************************************************/
ExternalExtensions::ExternalExtension::ExternalExtension(const QString &path,
                                     const QString &id,
                                     const QString &name,
                                     const QString &author,
                                     const QString &version,
                                     const QString &trigg,
                                     const QStringList &dependencies)
    : QueryHandler(id),
      path(path),
      name(name),
      author(author),
      version(version),
      dependencies(dependencies),
      trigger_(trigg) {

    QProcess extProc;
    extProc.start(path, {"INITIALIZE"});
    extProc.waitForFinished(-1);

    QString reply = extProc.readAllStandardOutput();
    if (!reply.isEmpty())
        throw QString("Initialization failed: %1").arg(reply);

}


/** ***************************************************************************/
ExternalExtensions::ExternalExtension::~ExternalExtension() {
    QProcess extProc;
    extProc.start(path, {"FINALIZE"});
    extProc.waitForFinished(-1);
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtension::setupSession() {
    QProcess extProc;
    extProc.start(path, {"SETUPSESSION"});
    extProc.waitForFinished(-1);
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtension::teardownSession() {
    QProcess extProc;
    extProc.start(path, {"TEARDOWNSESSION"});
    extProc.waitForFinished(-1);
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtension::handleQuery(Query* query) {

    QProcess extProc;
    extProc.start(path, {"QUERY", query->searchTerm()});
    extProc.waitForFinished(-1);

    vector<pair<shared_ptr<Core::Item>,short>> results;
    QJsonDocument document = QJsonDocument::fromJson(extProc.readAllStandardOutput());
    QJsonArray array = document.array();

    // Iterate over the results
    shared_ptr<StandardItem> item;
    shared_ptr<StandardAction> action;
    for (const QJsonValue & value : array){
        QJsonObject obj = value.toObject();

        QString id = obj["id"].toString();
        if (!id.isEmpty()){

            item = std::make_shared<StandardItem>(id);
            item->setText(obj["name"].toString());
            item->setSubtext(obj["description"].toString());
            QString path;
            if ( !(path = XdgIconLookup::instance()->themeIconPath(obj["icon"].toString())).isNull() )
                item->setIconPath(path);
            else if ( !(path = XdgIconLookup::instance()->themeIconPath("unknown")).isNull() )
                item->setIconPath(path);
            else
                item->setIconPath(":unknown");

            // Build the actions
            QJsonArray jsonActions = obj["actions"].toArray();
            vector<shared_ptr<Action>> actions;
            for (const QJsonValue & value : jsonActions){
                QJsonObject obj = value.toObject();
                action = std::make_shared<StandardAction>(); // Todo make std commadn action
                action->setText(obj["name"].toString());
                QString command = obj["command"].toString();
                QStringList arguments;
                for (const QJsonValue & value : obj["arguments"].toArray())
                     arguments.append(value.toString());
                action->setAction([command, arguments](){
                    QProcess::startDetached(command, arguments);
                });
                actions.push_back(action);
            }
            item->setActions(std::move(actions));

            results.emplace_back(std::move(item), 0);
        }
    }

    query->addMatches(results.begin(), results.end());
}


