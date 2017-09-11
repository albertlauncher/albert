// albert extension mpris - a mpris interface plugin for albert
// Copyright (C) 2016-2017 Martin Buergmann
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

#include "extension.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QMap>
#include <QPointer>
#include <QStringList>
#include "core/query.h"
#include "xdg/iconlookup.h"
#include "command.h"
#include "configwidget.h"
#include "player.h"

#define themeOr(name, fallbk)   XDG::IconLookup::iconPath(name).isEmpty() ? fallbk : XDG::IconLookup::iconPath(name)

namespace  {
static const int DBUS_TIMEOUT = 25 /* ms */;
}

class MPRIS::Private
{
public:
    ~Private();

    const char* name = "MPRIS Control";
    static QDBusMessage findPlayerMsg;
    QPointer<MPRIS::ConfigWidget> widget;
    QList<MPRIS::Player *> mediaPlayers;
    QStringList commands;
    QMap<QString, MPRIS::Command> commandObjects;


    QDBusMessage call(QDBusMessage &toDispatch);

};


QDBusMessage MPRIS::Private::findPlayerMsg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListNames");



/** ***************************************************************************/
MPRIS::Private::~Private() {
    // If there are still media player objects, delete them
    qDeleteAll(mediaPlayers);
    // Don't need to destruct the command objects.
    // This is done by the destructor of QMap
}



/** ***************************************************************************/
QDBusMessage MPRIS::Private::call(QDBusMessage &toDispatch) {
    return QDBusConnection::sessionBus().call(toDispatch, QDBus::Block, DBUS_TIMEOUT);
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
MPRIS::Extension::Extension()
    : Core::Extension("org.albert.extension.mpris"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    QString icon;

    // Setup the DBus commands
    icon = themeOr("media-playback-start", ":play");
    Command* nextToAdd = new Command(
                "play", // Label
                "Play", // Title
                "Start playing on %1", // Subtext
                "Play", // DBus Method
                icon
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", false);
    d->commands.append("play");
    d->commandObjects.insert("play", *nextToAdd);

    icon = themeOr("media-playback-pause", ":pause");
    nextToAdd = new Command(
                "pause",
                "Pause",
                "Pause %1",
                "Pause",
                icon
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", true);
    d->commands.append("pause");
    d->commandObjects.insert("pause", *nextToAdd);

    icon = themeOr("media-playback-stop", ":stop");
    nextToAdd = new Command(
                "stop",
                "Stop",
                "Stop %1",
                "Stop",
                icon
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", true);
    d->commands.append("stop");
    d->commandObjects.insert("stop", *nextToAdd);

    icon = themeOr("media-skip-forward", ":next");
    nextToAdd = new Command(
                "next track",
                "Next track",
                "Play next track on %1",
                "Next",
                icon
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoNext", true, true);
    //.fireCallback([](){qInfo("NEXT");})
    d->commands.append("next track");
    d->commandObjects.insert("next track", *nextToAdd);

    icon = themeOr("media-skip-backward", ":prev");
    nextToAdd = new Command(
                "previous track",
                "Previous track",
                "Play previous track on %1",
                "Previous",
                icon
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoPrevious", true, true);
    d->commands.append("previous track");
    d->commandObjects.insert("previous track", *nextToAdd);
}



/** ***************************************************************************/
MPRIS::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *MPRIS::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}



/** ***************************************************************************/
void MPRIS::Extension::setupSession() {

    // Clean the memory
    qDeleteAll(d->mediaPlayers);
    d->mediaPlayers.clear();

    // If there is no session bus, abort
    if (!QDBusConnection::sessionBus().isConnected())
        return;

    // Querying the DBus to list all available services
    QDBusMessage response = d->call(Private::findPlayerMsg);

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

                    for (QString& busId : busids) {

                        // Query the name of the media player of which we have the bus id.
                        QDBusInterface iface(busId, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2");
                        iface.setTimeout(DBUS_TIMEOUT);

                        QString name = busId;
                        QVariant prop = iface.property("Identity");
                        if (prop.isValid() && !prop.isNull() && prop.canConvert(QVariant::String)) {
                            name = prop.toString();
                        } else {
                            qWarning("DBus: Name is either invalid, null or not instanceof string");
                        }

                        bool canRaise = false;
                        prop = iface.property("CanRaise");
                        if (prop.isValid() && !prop.isNull() && prop.canConvert(QVariant::Bool)) {
                            canRaise = prop.toBool();
                        } else {
                            qWarning("DBus: CanRaise is either invalid, null or not instanceof bool");
                        }

                        // And add their player object to the list
                        d->mediaPlayers.push_back(new Player{busId, name, canRaise});

                    }


                } else {
                    qCritical("[%s] DBus error: Argument is either not type of QStringList or is empty!", d->name);
                }
            } else {
                qCritical("[%s] DBus error: Reply argument not valid or null!", d->name);
            }
        } else {
            qCritical("[%s] DBus error: Expected 1 argument for DBus reply. Got %d", d->name, args.length());
        }
    } else {
        qCritical("[%s] DBus error: %s", d->name, response.errorMessage().toStdString().c_str());
    }
}



/** ***************************************************************************/
void MPRIS::Extension::handleQuery(Core::Query *query) const {

    if ( query->searchTerm().isEmpty() )
        return;

    // Do not proceed if there are no players running. Why would you even?
    if (d->mediaPlayers.isEmpty())
        return;

    const QString& q = query->searchTerm().toLower();

    // Filter applicable commands
    QStringList cmds;
    for (QString& cmd : d->commands) {
        if (cmd.startsWith(q))
            cmds.append(cmd);
    }


    // For every option create entries for every player
    uint percentage = 0;
    for (QString& cmd: cmds) {
        // Calculate how many percent of the query match the command
        percentage = static_cast<uint>(1.0*q.length()/cmd.length())*UINT_MAX;

        // Get the command
        Command& toExec = d->commandObjects.find(cmd).value();
        // For every player:
        for (Player *p : d->mediaPlayers) {
            // See if it's applicable for this player
            if (toExec.isApplicable(*p))
                // And add a match if so
                query->addMatch(toExec.produceAlbertItem(*p), percentage);
        }
    }
}



/** ***************************************************************************/
QString MPRIS::Extension::name() const {
    return d->name;
}
