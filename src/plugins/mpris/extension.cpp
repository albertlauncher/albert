// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include "extension.h"
#include "configwidget.h"
#include "query.h"
#include "objects.hpp"
#include "albertapp.h"
#include "xdgiconlookup.h"
#include "command.h"

/** ***************************************************************************/
MPRIS::Extension::Extension() : IExtension("MPRIS Control Center") {
    qDebug("[%s] Initialize extension", name_);

    // Setup the DBus commands
    commands.append("play");
    commandObjects.insert("play",
                          Command(
                              "play",
                              "Start playing",
                              "Play",
                              XdgIconLookup::instance()->themeIconPath("media-playback-start.png", QIcon::themeName()))
                          .applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", QVariant("Playing"), false)
                          .closeWhenHit()
                          );

    commands.append("pause");
    commandObjects.insert("pause",
                          Command(
                              "pause",
                              "Pause",
                              "Pause",
                              XdgIconLookup::instance()->themeIconPath("media-playback-pause.png", QIcon::themeName()))
                          .applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", QVariant("Playing"), true)
                          .closeWhenHit()
                          );

    commands.append("stop");
    commandObjects.insert("stop",
                          Command(
                              "stop",
                              "Stop playing",
                              "Stop",
                              XdgIconLookup::instance()->themeIconPath("media-playback-stop.png", QIcon::themeName()))
                          .applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", QVariant("Playing"), true)
                          .closeWhenHit()
                          );

    commands.append("next");
    commandObjects.insert("next",
                          Command(
                              "next",
                              "Next track",
                              "Next",
                              XdgIconLookup::instance()->themeIconPath("media-skip-forward.png", QIcon::themeName()))
                          .applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoNext", QVariant(true), true)
                          //.fireCallback([](){qDebug("NEXT");})
                          );

    commands.append("previous");
    commandObjects.insert("previous",
                          Command(
                              "previous",
                              "Previous track",
                              "Previous",
                              XdgIconLookup::instance()->themeIconPath("media-skip-backward.png", QIcon::themeName()))
                          .applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoPrevious", QVariant(true), true)
                          );

    qDebug("[%s] Extension initialized", name_);
}



/** ***************************************************************************/
MPRIS::Extension::~Extension() {
    qDebug("[%s] Finalize extension", name_);
    // Do sth.
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
    // Querying the DBus to list all available services
    QDBusMessage findPlayerMsg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListNames");
    QDBusMessage response = QDBusConnection::sessionBus().call(findPlayerMsg);

    // Do some error checking
    if (response.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> args = response.arguments();
        if (args.length() == 1) {
            QVariant arg = args.at(0);
            if (!arg.isNull() && arg.isValid()) {
                QStringList names = arg.toStringList();
                if (!names.isEmpty()) {
                    // No errors

                    // Filter all mpris capable
                    names = names.filter(QRegExp("org\\.mpris\\.MediaPlayer2\\.(.*)"));
                    mediaPlayers.clear();
                    for (QString busid : names) {
                        // And add their player object to the list
                        mediaPlayers.append(Player(busid));
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
void MPRIS::Extension::teardownSession() {
}



/** ***************************************************************************/
bool MPRIS::Extension::pairsort(QPair<int, QString> left, QPair<int, QString> right) {
    return left.first < right.first;
}



/** ***************************************************************************/
void MPRIS::Extension::handleQuery(shared_ptr<Query> query) {
    // Do not proceed if there are no players running. Why would you even?
    if (mediaPlayers.isEmpty())
        return;

    // Filter all applicable commands
    QString q = query->searchTerm();
    QRegExp rx(q);
    QStringList possibleCommands = commands.filter(rx);


    // Sort them by their occurence index
    // If the user types 's'
    // 'stop' is before 'pause'
    // Both contains a 's' but it occures earlier in 'stop'
    QList<QPair<int, QString>> cmds;
    for (QString& str: possibleCommands)
        cmds.append(QPair<int, QString>(rx.indexIn(str), str));
    qSort(cmds.begin(), cmds.end(), &pairsort);


    // For every option create entries for every player
    for (QPair<int, QString>& cmd: cmds) {
        Command& toExec = commandObjects.find(cmd.second).value();
        for (Player p: mediaPlayers) {
            if (toExec.isApplicable(p))
                query->addMatch(toExec.produceAlbertItem(p), SHRT_MAX);
        }
    }
}



/** ***************************************************************************/
void MPRIS::Extension::handleFallbackQuery(shared_ptr<Query> query) {
    // Avoid annoying warnings
    Q_UNUSED(query)
}
