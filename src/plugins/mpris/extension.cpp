// albert extension mpris - a mpris interface plugin for albert
// Copyright (C) 2016 Martin Buergmann
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
#include "extension.h"
#include "configwidget.h"
#include "query.h"
#include "objects.hpp"
#include "albertapp.h"
#include "xdgiconlookup.h"
#include "command.h"

//QRegExp MPRIS::Extension::filterRegex("org\\.mpris\\.MediaPlayer2\\.(.*)");
QDBusMessage MPRIS::Extension::findPlayerMsg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListNames");

/** ***************************************************************************/
MPRIS::Extension::Extension() : IExtension("MPRIS Control Center") {
    qDebug("[%s] Initialize extension", name_);

    // Local cache field
    QString iconThemeName = QIcon::themeName();
    XdgIconLookup* iconlookup = XdgIconLookup::instance();

    // Setup the DBus commands
    Command* nextToAdd = new Command(
                "play", // Label
                "Start playing", // Title
                "Play", // DBus Method
                iconlookup->themeIconPath("media-playback-start", iconThemeName)
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", false);
    nextToAdd->closesWhenHit();
    commands.append("play");
    commandObjects.insert("play", *nextToAdd);

    nextToAdd = new Command(
                "pause",
                "Pause",
                "Pause",
                iconlookup->themeIconPath("media-playback-pause", iconThemeName));
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", true);
    nextToAdd->closeWhenHit();
    commands.append("pause");
    commandObjects.insert("pause", *nextToAdd);

    nextToAdd = new Command(
                "stop",
                "Stop playing",
                "Stop",
                iconlookup->themeIconPath("media-playback-stop", iconThemeName));
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", true);
    nextToAdd->closeWhenHit();
    commands.append("stop");
    commandObjects.insert("stop", *nextToAdd);

    nextToAdd = new Command(
                "next",
                "Next track",
                "Next",
                iconlookup->themeIconPath("media-skip-forward", iconThemeName));
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoNext", true, true);
    //.fireCallback([](){qDebug("NEXT");})
    commands.append("next");
    commandObjects.insert("next", *nextToAdd);

    nextToAdd = new Command(
                "previous",
                "Previous track",
                "Previous",
                iconlookup->themeIconPath("media-skip-backward", iconThemeName));
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoPrevious", true, true);
    commands.append("previous");
    commandObjects.insert("previous", *nextToAdd);

    qDebug("[%s] Extension initialized", name_);
}



/** ***************************************************************************/
MPRIS::Extension::~Extension() {
    qDebug("[%s] Finalize extension", name_);

    // If there are still media player objects, delete them
    if (!mediaPlayers.isEmpty()) {
        for (Player* p: mediaPlayers) {
            delete p;
        }
    }

    // Don't need to destruct the command objects.
    // This is done by the destructor of QMap

    qDebug("[%s] Extension finalized", name_);
}



/** ***************************************************************************/
QWidget *MPRIS::Extension::widget(QWidget *parent) {
    if (widget_.isNull()) {
        widget_ = new ConfigWidget(parent);
    }
    return widget_;
}



/** ***************************************************************************/
void MPRIS::Extension::setupSession() {

    // Clean the memory
    for (Player* p: mediaPlayers) {
        delete p;
    }
    mediaPlayers.clear();

    // If there is no session bus, abort
    if (!QDBusConnection::sessionBus().isConnected())
        return;

    // Querying the DBus to list all available services
    QDBusMessage response = QDBusConnection::sessionBus().call(findPlayerMsg);

    // Do some error checking
    if (response.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> args = response.arguments();
        if (args.length() == 1) {
            QVariant arg = args.at(0);
            if (!arg.isNull() && arg.isValid()) {
                QStringList runningBusEndpoints = arg.toStringList();
                if (!runningBusEndpoints.isEmpty()) {
                    // No errors

                    // Filter all mpris capable
                    //names = names.filter(filterRegex);
                    QStringList busids;
                    for (QString& id: runningBusEndpoints) {
                        if (id.startsWith("org.mpris.MediaPlayer2."))
                            busids.append(id);
                    }

                    for (QString& busid : busids) {
                        // And add their player object to the list
                        mediaPlayers.append(new Player(busid));
                    }


                } else {
                    qCritical("[%s] DBus error: Argument is either not type of QStringList or is empty!", name_);
                }
            } else {
                qCritical("[%s] DBus error: Reply argument not valid or null!", name_);
            }
        } else {
            qCritical("[%s] DBus error: Expected 1 argument for DBus reply. Got %d", name_, args.length());
        }
    } else {
        qCritical("[%s] DBus error: %s", name_, response.errorMessage().toStdString().c_str());
    }
}



/** ***************************************************************************/
void MPRIS::Extension::handleQuery(shared_ptr<Query> query) {
    // Do not proceed if there are no players running. Why would you even?
    if (mediaPlayers.isEmpty())
        return;

    const QString& q = query->searchTerm();

    // Filter applicable commands
    QStringList cmds;
    for (QString& cmd : commands) {
        if (cmd.startsWith(q))
            cmds.append(cmd);
    }


    // For every option create entries for every player
    short percentage = 0;
    for (QString& cmd: cmds) {
        // Calculate how many percent of the query match the command
        percentage = (float)q.length() / (float)cmd.length() *100;

        // Get the command
        Command& toExec = commandObjects.find(cmd).value();
        // For every player:
        for (Player* p: mediaPlayers) {
            // See if it's applicable for this player
            if (toExec.isApplicable(*p))
                // And add a match if so
                query->addMatch(toExec.produceAlbertItem(*p), percentage);
        }
    }
}
