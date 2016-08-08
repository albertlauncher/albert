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

#include <QLabel>
#include <QProcess>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include "externalextension.h"
#include "standarditem.hpp"
#include "standardaction.hpp"
#include "xdgiconlookup.h"

using std::vector;

namespace  {
    vector<SharedItem> buildItemFromJson(const QByteArray &a){

        vector<SharedItem> result;
        QJsonDocument document = QJsonDocument::fromJson(a);
        QJsonArray array = document.array();

        // Iterate over the results
        SharedStdItem item;
        SharedStdAction action;
        for (const QJsonValue & value : array){
            QJsonObject obj = value.toObject();

            /* Build the actions */

            QJsonArray jsonActions = obj["actions"].toArray();
            vector<SharedAction> actions;
            for (const QJsonValue & value : jsonActions){
                QJsonObject obj = value.toObject();
                action = std::make_shared<StandardAction>(); // Todo make std commadn action
                action->setText(obj["name"].toString());
                QString command = obj["command"].toString();
                QStringList arguments;
                for (const QJsonValue & value : obj["arguments"].toArray())
                     arguments.append(value.toString());
                action->setAction([command, arguments](ExecutionFlags *){
                    QProcess::startDetached(command, arguments);
                });
                actions.push_back(action);
            }

            /* Build the item*/

            QString id =  obj["id"].toString();
            if (!id.isEmpty()){
            // If icon is not a valid path try icon lookup
                QString iconPath = obj["icon"].toString();
                iconPath = XdgIconLookup::instance()->themeIconPath(iconPath);
                result.emplace_back(new StandardItem(id,
                                                     obj["name"].toString(),
                                                     obj["description"].toString(),
                                                     iconPath,
                                                     actions));
            }
        }
        return result;
    }
}



/** ***************************************************************************/
ExternalExtension::ExternalExtension(const char *id, QString path)
    : AbstractExtension(id), path_(path) {

    // Get Metadata
    QProcess extProc;
    extProc.start(path, {"METADATA"});
    if (!extProc.waitForFinished(1000))
        throw QString("Communication timed out. (METADATA)");

    // Read JSON data
    QJsonDocument doc = QJsonDocument::fromJson(extProc.readAllStandardOutput());
    if (doc.isNull() || !doc.isObject())
        throw QString("Reply to 'METADATA' is not a valid JSON object.");

    QJsonObject metadata = doc.object();

    providesMatches_   = metadata["providesMatches"].toBool(false);
    providesFallbacks_ = metadata["providesFallbacks"].toBool(false);
    runTriggeredOnly_  = metadata["runTriggeredOnly"].toBool(false);

    // If RunTriggeredOnly is set check for triggers
    if (runTriggeredOnly_) {
        for (const QJsonValue & value : metadata["triggers"].toArray())
            triggers_.append(value.toString());
        if (triggers_.isEmpty())
            throw QString("No triggers set.");
    }

    // Try running the initialization
    extProc.start(path_, {"INITIALIZE"});
    if (!extProc.waitForFinished(10000))
        throw QString("Communication timed out. (INITIALIZE)");
    QString reply = extProc.readAllStandardOutput();
    if (!reply.isEmpty())
        throw QString("Initialization failed: %1").arg(reply);
}



/** ***************************************************************************/
ExternalExtension::~ExternalExtension() {
    QProcess::startDetached(path_, {"FINALIZE"});
}



/** ***************************************************************************/
QString ExternalExtension::name() const {
    QProcess extProc;
    extProc.start(path_, {"NAME"});
    if (!extProc.waitForFinished(5000))
        return QString();
    return QString(extProc.readAllStandardOutput());
}



/** ***************************************************************************/
QWidget *ExternalExtension::widget(QWidget *parent) {

    QWidget *w = new QWidget(parent);
    QVBoxLayout *vl = new QVBoxLayout(w);
    QLabel* label = new QLabel(w);

    vl->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vl->addWidget(label);
    vl->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    label->setAlignment(Qt::AlignCenter);
    label->setText("<html><head/><"
                   "body>"
                   "<p><span style=\" color:#808080;\">This is an external extension.</span></p>"
                   "<p><span style=\" color:#808080;\">Currently external extensions are not configurable.</span></p>"
                   "</body>"
                   "</html>");

    return w;
}



/** ***************************************************************************/
bool ExternalExtension::runExclusive() const {
     return runTriggeredOnly_;
}



/** ***************************************************************************/
QStringList ExternalExtension::triggers() const {
    return triggers_;
}



/** ***************************************************************************/
void ExternalExtension::setupSession() {
    QProcess extProc;
    extProc.start(path_, {"SETUPSESSION"});
    if (!extProc.waitForFinished(1000))
        return;
}



/** ***************************************************************************/
void ExternalExtension::teardownSession() {
    QProcess extProc;
    extProc.start(path_, {"TEARDOWNSESSION"});
    if (!extProc.waitForFinished(1000))
        return;
}



/** ***************************************************************************/
void ExternalExtension::handleQuery(Query query) {

    if (!providesMatches_)
        return;

    QProcess extProc;
    extProc.start(path_, {"QUERY", query.searchTerm()});
    if (!extProc.waitForFinished(1000))
        return;
    for (SharedItem &item : buildItemFromJson(extProc.readAllStandardOutput()))
        query.addMatch(item);
}



/** ***************************************************************************/
vector<SharedItem> ExternalExtension::fallbacks(QString query) const {

    if (!providesFallbacks_)
        return vector<SharedItem>();

    QProcess extProc;
    extProc.start(path_, {"FALLBACKS", query});
    if (!extProc.waitForFinished(1000))
        return vector<SharedItem>();
    return buildItemFromJson(extProc.readAllStandardOutput());
}


