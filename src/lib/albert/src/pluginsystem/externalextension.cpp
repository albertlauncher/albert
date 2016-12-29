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

namespace  {
    vector<shared_ptr<Item>> buildItemFromJson(const QByteArray &a){

        vector<shared_ptr<Item>> result;
        QJsonDocument document = QJsonDocument::fromJson(a);
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
                item->setIconPath(XdgIconLookup::instance()->themeIconPath(obj["icon"].toString()));

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

                result.emplace_back(std::move(item));
            }
        }
        return result;
    }
}



/** ***************************************************************************/
Core::ExternalExtension::ExternalExtension(const char *id, QString path)
    : Extension(id), path_(path) {

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
    trigger_ = metadata["trigger"].toString();

    // Try running the initialization
    extProc.start(path_, {"INITIALIZE"});
    if (!extProc.waitForFinished(10000))
        throw QString("Communication timed out. (INITIALIZE)");
    QString reply = extProc.readAllStandardOutput();
    if (!reply.isEmpty())
        throw QString("Initialization failed: %1").arg(reply);
}



/** ***************************************************************************/
Core::ExternalExtension::~ExternalExtension() {
    QProcess::startDetached(path_, {"FINALIZE"});
}



/** ***************************************************************************/
QString Core::ExternalExtension::name() const {
    QProcess extProc;
    extProc.start(path_, {"NAME"});
    if (!extProc.waitForFinished(5000))
        return QString();
    return QString(extProc.readAllStandardOutput());
}



/** ***************************************************************************/
QWidget *Core::ExternalExtension::widget(QWidget *parent) {

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
QString Core::ExternalExtension::trigger() const {
    return trigger_;
}



/** ***************************************************************************/
void Core::ExternalExtension::setupSession() {
    QProcess::startDetached(path_, {"SETUPSESSION"});
}



/** ***************************************************************************/
void Core::ExternalExtension::teardownSession() {
    QProcess::startDetached(path_, {"TEARDOWNSESSION"});
}



/** ***************************************************************************/
void Core::ExternalExtension::handleQuery(Query* query) {

    QProcess extProc;
    extProc.start(path_, {"QUERY", query->searchTerm()});
    if (!extProc.waitForFinished(1000))
        return;
    for (shared_ptr<Item> &item : buildItemFromJson(extProc.readAllStandardOutput()))
        query->addMatch(item);
}


